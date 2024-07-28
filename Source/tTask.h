#ifndef __TTASK_H
#define __TTASK_H

#include "tLib.h"

typedef uint32_t tTaskStack;
typedef unsigned long ul;

typedef enum _TINYOS_TASK_STATE{
	TASK_READY 		= 	0,
	TASK_DELAY 		= 	1 << 1,
	TASK_SUSPEND 	= 	1 << 2,
	TASK_DELETE 	= 	1 << 3
}TINYOS_TASK_STATE;

typedef struct _tTask {
	tTaskStack * stack;						// 任务栈空间
	uint32_t *stackBase;					// 栈的起始地址
	uint32_t stackSize;						// 栈的大小
	tNode linkNode;							// 时间片轮转节点
	tNode delayNode;						// 延时节点
	uint32_t delayTicks;					// 延时时长
	uint32_t suspendCount;					// 挂起次数
	uint32_t state;							// 任务状态
	uint32_t slice;							// 时间片长度
	uint8_t prio; 							// 任务优先级
	
	void (*clean) (void* param); 			// 删除回调函数
	void* cleanParam;						// 删除回调函数的参数
	uint8_t requestDeleteFlag; 				// 删除请求标志位
	
	void* waitEvent;						// 任务所在的等待队列
	void* eventMsg;							// 事件消息
	uint32_t waitEventResult;				// 等待时间的结果
	
	uint32_t waitFlagsType;
	uint32_t eventFlags;
}tTask;

typedef struct _tTaskInfo
{
	uint32_t delayTicks;
	uint32_t prio;
	uint32_t state;
	uint32_t slice;
	uint32_t suspendCount;
	uint32_t stackSize;
	uint32_t freeSize;
}tTaskInfo;

extern tTask *currentTask;
extern tTask *nextTask;


// 调度
// 任务初始化
void tTask_Systick_config(uint32_t ms);
void tTaskInit(tTask* task, void (* entry) (void *), void * param, uint8_t prio, uint32_t slice, tTaskStack* stack, uint32_t size);
void tTaskSched(void);
void tTaskSchedInit(void);
void tTask_Sched_Enable(void);
void tTask_Sched_Disable(void);
tTask* tTask_HighPrio_Ready(void);
// 挂起
void tTaskSuspend(tTask* task);
void tTaskWakeSuspend(tTask* task);
// 删除		-- 		分两步 从队列(就绪，延时)中删除，清理资源(调用回调)
void tTaskSchedRemove(tTask* task);
void tTimeTaskRemove(tTask* task);
void tTaskSetCleanCallFunc(tTask* task, void (*clean) (void* param), void* param);
void tTaskForceDelete(tTask* task);
void tTaskSetDeleteRequest(tTask* task);
uint8_t tTaskIsRequestDeleted(tTask* task);
void tTaskDeleteSelf(void);
void tTaskDeleteNode(tTask* task);
// 获取任务信息
void tTaskGetInfo(tTask* task, tTaskInfo* info);
// 任务调度列表
void tTaskSchedRdy(tTask* task);
void tTaskSchedUnRdy(tTask* task);
// 中断开关
uint32_t tTask_Enter_Critical(void);
void tTask_Exit_Critical(uint32_t status);

#endif
