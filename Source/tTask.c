#include "tinyOS.h"
#include "tConfig.h"
#include "tLib.h"
#include "ARMCM3.h"
#include "string.h"


// 调度保护变量
uint8_t tTaskLockSched = 0;


// 调度资源
tBitmap bitmap;
tList taskTable[TINYOS_PRIO_COUNT];

// 调度资源
tTask *currentTask;
tTask *nextTask;
tTask *idle;

/**
	\brief        	任务资源初始化
    \param[in]  	执行函数
    \param[in]  	执行函数的参数
	\param[in] 		任务运行的栈顶
     
    \param[out] none
    \retval     none
*/
void tTaskInit(tTask* task, void (* entry) (void *), void * param, uint8_t prio, uint32_t slice, tTaskStack* stack, uint32_t size)
{
	uint32_t *stackTop;
	
	task->stackBase = stack;
	task->stackSize = size;
	memset(stack, 0, size);
	
	stackTop = stack + size / sizeof(tTaskStack);
	// 硬件自动保存的寄存器
	*(--stackTop) = (ul)(1 << 24);			// xPSR
	*(--stackTop) = (ul)entry;				// PC -- R15
	*(--stackTop) = (ul)0x14;				// LR -- R14
	*(--stackTop) = (ul)0x12;				// R12
	*(--stackTop) = (ul)0x3;				// R3
	*(--stackTop) = (ul)0x2;				// R2
	*(--stackTop) = (ul)0x1;				// R1
	*(--stackTop) = (ul)param;				// R0
	// 手动保存的寄存器
	*(--stackTop) = (ul)0x11;
	*(--stackTop) = (ul)0x10;
	*(--stackTop) = (ul)0x9;
	*(--stackTop) = (ul)0x8;
	*(--stackTop) = (ul)0x7;
	*(--stackTop) = (ul)0x6;
	*(--stackTop) = (ul)0x5;
	*(--stackTop) = (ul)0x4;
	// 保存栈顶地址
	task->stack = stackTop;										// 任务栈
	task->delayTicks = 0;										// 延时
	task->prio = prio;											// 优先级
	task->state = TASK_READY;									// 任务状态
	task->slice = slice;										// 时间片长度
	task->suspendCount = 0;										// 挂起次数
	task->clean = (void (*) (void*)) 0; 						// 删除函数回调
	task->cleanParam = (void*) 0;								// 回调函数的参数
	task->requestDeleteFlag = 0;								// 删除请求
	tNodeInit(&(task->delayNode));								// 初始化延时节点
	tNodeInit(&(task->linkNode));								// 初始化任务链接节点
	tTaskSchedRdy(task);										// 将任务加入到就绪队列
}

// 系统时钟中断中的任务处理函数 -- 完成调度前的延时列表遍历以及时间片递减操作
void tTaskSystemTickHandler(void)
{
	// 遍历延时列表
	tTimeKeep();
	// 判断时间片
	if(--(currentTask->slice) == 0)
	{
		if(tListCount(&taskTable[currentTask->prio]) > 0)
		{
			tListRemoveFirst(&taskTable[currentTask->prio]);
			tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));
			
			currentTask->slice = TINYOS_SLICE_MAX;
		}
	}
	ticktop();
	checkCpuUsage();
	tTimerModuleTickNotify();
	tTaskSched();
}


// 进入临界区(关中断)
uint32_t tTask_Enter_Critical(void)
{
	uint32_t primask = __get_PRIMASK(); // 读取中断开关寄存器
	__disable_irq(); // 关闭中断
	return primask;
}
// 退出临界区(恢复进入临界区前的状态)
void tTask_Exit_Critical(uint32_t status)
{
	__set_PRIMASK(status);
}
// 进入临界区(关调度)
void tTask_Sched_Disable(void)
{
	uint32_t status = tTask_Enter_Critical();
	if(tTaskLockSched < 255) tTaskLockSched++;
	tTask_Exit_Critical(status);
}
// 退出临界区(开启调度)
void tTask_Sched_Enable(void)
{
	uint32_t status = tTask_Enter_Critical();
	if(tTaskLockSched > 0)
		if(--tTaskLockSched == 0)
			tTaskSched();
	tTask_Exit_Critical(status);
}

// 初始化调度变量
void tTaskSchedInit(void)
{
	tTaskLockSched = 0;
	tBitmapInit(&bitmap);
	for(int i = 0; i < TINYOS_PRIO_COUNT; i++)
	{
		tListInit(&taskTable[i]);
	}
}

// 获取可调度的高优先级任务的指针
tTask* tTask_HighPrio_Ready(void)
{
	uint8_t pos = tBitmapGetFirstSet(&bitmap);
	tNode* node = tListFirst(&taskTable[pos]);
	return tNodeParent(node, tTask, linkNode);
}



// 简单的调度函数
void tTaskSched(void)
{
	// 进行调度时要关掉中断
	uint32_t status = tTask_Enter_Critical();
	// 当信号量大于0时停止调度
	if(tTaskLockSched > 0) 
	{
		tTask_Exit_Critical(status);
		return;
	}
	// 切换任务
	tTask* tmpTask = tTask_HighPrio_Ready();
	if(currentTask != tmpTask)
	{
		nextTask = tmpTask;
		// 触发PENDSV 切换上下文
		tTaskSwitch();
	}
	tTask_Exit_Critical(status);
}

// 加入调度
void tTaskSchedRdy(tTask* task)
{
	uint32_t status = tTask_Enter_Critical();
	tListAddLast(&taskTable[task->prio], &(task->linkNode));
	tBitmapSet(&bitmap, task->prio);
	tTask_Exit_Critical(status);
}
// 退出调度
void tTaskSchedUnRdy(tTask* task)
{
	uint32_t status = tTask_Enter_Critical();
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&bitmap, task->prio);
	}
	tTask_Exit_Critical(status); 
}

// 将任务挂起
void tTaskSuspend(tTask* task)
{
	uint32_t status = tTask_Enter_Critical();
	
	if((task->state & TASK_DELAY) == 0)
	{
		if(++(task->suspendCount) <= 1)
		{
			// 将任务从就绪列表中删除，并设置任务为挂起状态
			task->state |= TASK_SUSPEND;
			tTaskSchedUnRdy(task);
			// 如果当前任务正在运行则执行调度
			if(task == currentTask) tTaskSched();
		}
	}
	tTask_Exit_Critical(status);
}


// 唤醒挂起的任务
void tTaskWakeSuspend(tTask* task)
{
	uint32_t status = tTask_Enter_Critical();
	if(task->state & TASK_SUSPEND)
	{
		if(--(task->suspendCount) == 0)
		{
			task->state &= ~TASK_SUSPEND;
			tTaskSchedRdy(task);
			tTaskSched();
		}
	}
	tTask_Exit_Critical(status);
}

// 将任务从就绪队列中移除
void tTaskSchedRemove(tTask* task)
{
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if(tListCount(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&bitmap, task->prio);
	}
}

// 清理的回调函数
void tTaskSetCleanCallFunc(tTask* task, void (*clean) (void* param), void* param)
{
	task->clean = clean;
	task->cleanParam = param;
}  

// 强制删除任务 -- 不管任务处于什么状态都要删除
void tTaskForceDelete(tTask* task)
{
	uint32_t status = tTask_Enter_Critical();
	if(task->state & TASK_DELAY) tTimeTaskRemove(task);
	else if(!(task->state & TASK_SUSPEND)) tTaskSchedRemove(task);
	
	if(task->clean) task->clean(task->cleanParam);
	if(currentTask == task) tTaskSched();
	tTask_Exit_Critical(status);
}

// 发起清理请求
void tTaskSetDeleteRequest(tTask* task)
{
	uint32_t status = tTask_Enter_Critical();
	task->requestDeleteFlag = 1;
	tTask_Exit_Critical(status);
}

// 获取清理请求标志位
uint8_t tTaskIsRequestDeleted(tTask* task)
{
	uint8_t is_delete = 0;
	uint32_t status = tTask_Enter_Critical();
	is_delete = task->requestDeleteFlag;
	tTask_Exit_Critical(status);
	return is_delete;
}

// 删除当前运行的任务
void tTaskDeleteSelf(void)
{
	uint32_t status = tTask_Enter_Critical();
	tTaskSchedRemove(currentTask);
	if(currentTask->clean) currentTask->clean(currentTask->cleanParam);
	tTaskSched();
	tTask_Exit_Critical(status);
}

// 获取任务的信息
void tTaskGetInfo(tTask* task, tTaskInfo* info)
{
	uint32_t *stackEnd;
	uint32_t status = tTask_Enter_Critical();
	info->prio = task->prio;
	info->slice = task->slice;
	info->state = task->state;
	info->delayTicks = task->delayTicks;
	info->suspendCount = task->suspendCount;
	
	info->stackSize = task->stackSize;
	info->freeSize = 0;
	stackEnd = task->stackBase;
	while((*(stackEnd++)) == 0 && (stackEnd <= (task->stackBase + task->stackSize / sizeof(tTaskStack))))
	{
		info->freeSize++;
	}
	info->freeSize *= sizeof(tTaskStack);
	
	tTask_Exit_Critical(status);
}

