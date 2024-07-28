#include "tinyOS.h"



/* 	
	硬件定时器在系统时钟处理函数中运行
	硬件定时器不需要管任务调度
	因此硬件定时器更准时，但是会影响调度效率
	软件定时器的执行会受软件调度的影响
	实时性要更差，但是不影响调度效率
	综上，
	硬件定时任务中不能添加高运行时长的任务，但实时性强
	软件定时器中则能够添加高运行时长的任务，实时性较差
*/
static tList tTimerHardList;
static tList tTimerSoftList;
static tSem tTimerProtectSem;
static tSem tTimerTickSem;

static void tTimerCallFuncList(tList* timerList);
static void tTimerSoftTask(void* param);


void tTimerInit(tTimer *timer, uint32_t delayTicks, uint32_t durationTicks,
	void (*timerFunc) (void *arg), void* arg, uint32_t config)
{
	tNodeInit(&timer->linkNode);
	timer->startDelayTicks = delayTicks;	// 起始延时时长
	timer->durationTicks = durationTicks; // 周期
	timer->timerFunc = timerFunc;
	timer->arg = arg;
	timer->config = config;
	
	// 如果设置了起始延时事件则使用，否则使用周期时间作为起始延时时长
	if(delayTicks == 0)
		timer->delayTicks = durationTicks;
	else
		timer->delayTicks = timer->startDelayTicks;
	timer->state = tTimerCreated;
}

static tTask tTimerTask;
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE];

static void tTimerSoftTask(void* param)
{
	while(1)
	{
		// 等待时间片唤醒
		tSemWait(&tTimerTickSem, 0);
		tSemWait(&tTimerProtectSem, 0);
		
		tTimerCallFuncList(&tTimerSoftList);
		
		tSemNotify(&tTimerProtectSem);
	}
}

void tTimerModuleTickNotify(void)
{
	uint32_t status = tTask_Enter_Critical();
	
	tTimerCallFuncList(&tTimerHardList);
	
	tTask_Exit_Critical(status);
	
	tSemNotify(&tTimerTickSem);
}

// 初始化软件定时器的相关资源
void tTimerModuleInit(void)
{
	// 初始化硬软件的任务列表
	tListInit(&tTimerHardList);
	tListInit(&tTimerSoftList);
	// 初始化滴答信号量和软件任务列表访问信号量
	tSemInit(&tTimerProtectSem, 1, 1);
	tSemInit(&tTimerTickSem, 0, 0);
	
#if	TINYOS_TIMERTASK_PRIO >= (TINYOS_PRIO_COUNT - 1)
	#error "The proprity of timer tasker must be greater than (TINYOS_PRO_COUNT - 1)"
#endif
	// 初始化任务
	tTaskInit(&tTimerTask, tTimerSoftTask, (void*)0, TINYOS_TIMERTASK_PRIO, 10, tTimerTaskStack, sizeof(tTimerTaskStack));
}

// 只初始化定时器任务
void tTimerInitTask(void)
{
#if	TINYOS_TIMERTASK_PRIO >= (TINYOS_PRIO_COUNT - 1)
	#error "The proprity of timer tasker must be greater than (TINYOS_PRO_COUNT - 1)"
#endif
	// 初始化任务
	tTaskInit(&tTimerTask, tTimerSoftTask, (void*)0, TINYOS_TIMERTASK_PRIO, 10, tTimerTaskStack, sizeof(tTimerTaskStack));

}

void tTimerStart(tTimer* timer)
{
	switch(timer->state)
	{
		case tTimerCreated:
		case tTimerStopped:
			timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
			timer->state = tTimerStarted;
			// 硬件定时器并不是硬件外设的定时器，仅仅指的是可在中断调用的定时器
			// 如果是硬件定时器，那么该任务列表可以在中断中使用，那么就需要屏蔽中断进行访问
			if(timer->config & TIMER_CONFIG_TYPE_HARD)
			{
				uint32_t status = tTask_Enter_Critical();
				tListAddFirst(&tTimerHardList, &(timer->linkNode));
				tTask_Exit_Critical(status);
			}
			else // 如果是软件定时器，则只能在任务调度中进行使用
			{
				tSemWait(&tTimerProtectSem, 0);
				tListAddFirst(&tTimerSoftList, &(timer->linkNode));
				tSemNotify(&tTimerProtectSem);
			}
			break;
		default:
			break;
	}
}

void tTimerStop(tTimer* timer)
{
	switch(timer->state)
	{
		case tTimerStarted:
		case tTimerRunning:
			// 硬件定时器并不是硬件外设的定时器，仅仅指的是可在中断调用的定时器
			// 如果是硬件定时器，那么该任务列表可以在中断中使用，那么就需要屏蔽中断进行访问
			if(timer->config & TIMER_CONFIG_TYPE_HARD)
			{
				uint32_t status = tTask_Enter_Critical();
				tListRemove(&tTimerHardList, &(timer->linkNode));
				tTask_Exit_Critical(status);
			}
			else // 如果是软件定时器，则只能在任务调度中进行使用
			{
				tSemWait(&tTimerProtectSem, 0);
				tListRemove(&tTimerSoftList, &(timer->linkNode));
				tSemNotify(&tTimerProtectSem);
			}
			timer->state = tTimerStopped;
			break;
		default:
			break;
	}
}	

static void tTimerCallFuncList(tList* timerList)
{
	tNode* node;
	for(node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
	{
		tTimer* timer = tNodeParent(node, tTimer, linkNode);
		// 如果起始计数器为0或者等待时钟时间已到那么就执行定时任务
		if((timer->delayTicks == 0) || (--timer->delayTicks == 0))
		{
			timer->state = tTimerRunning;
			timer->timerFunc(timer->arg);
			timer->state = tTimerStarted;
			
			if(timer->durationTicks == 0)
			{
				tListRemove(timerList, &(timer->linkNode));	
				timer->state = tTimerStopped;
			}
			else
			{
				timer->delayTicks = timer->durationTicks;
			}
		}
	}
}

// 删除定时器任务
void tTimerDestroy(tTimer* timer)
{
	tTimerStop(timer);
	timer->state = tTimerStopped;
}

// 获取定时器信息
void tTimerGetInfo(tTimer* timer, tTimerInfo* info)
{
	uint32_t status = tTask_Enter_Critical();
	info->startDelayTicks = timer->startDelayTicks;
	info->durationTicks = timer->durationTicks;
	info->timerFunc = timer->timerFunc;
	info->arg = timer->arg;
	info->config = timer->config;
	info->state = timer->state;
	tTask_Exit_Critical(status);
}