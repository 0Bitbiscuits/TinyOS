#include "tinyOS.h"

#define TASK_CLEAR_MASK (0x0000FFFF << 1)

void tEventInit(tEvent* event, tEventType type)
{
	event->type = type;
	tListInit(&(event->waitList));
}

void tEventWait(tEvent* event, tTask* task, void* msg, uint32_t state, uint32_t timeout)
{
	uint32_t status = tTask_Enter_Critical();
	
	// 配置任务状态
	task->state |= state;					// 状态
	task->waitEvent = event;				// 所等待的事件队列
	task->eventMsg = msg;					// 消息
	task->waitEventResult = tErrorNoError;	// 等待结果
	// 将任务从就绪队列中删除
	tTaskSchedUnRdy(task);
	// 先进先出 -- fifo
	tListAddLast(&(event->waitList), &(task->linkNode));
	// 如果设置了等待超时限制，那么就将任务添加到延时队列
	if(timeout) 
	{
		tTimeTaskWait(task, timeout);
	}
	
	tTask_Exit_Critical(status);
}

tTask* tEventWake(tEvent* event, void* msg, uint32_t result)
{
	tNode* node;
	tTask* task = (tTask*) 0;
	uint32_t status = tTask_Enter_Critical();
	
	// 取出等待队列中的第一个任务
	if((node = tListRemoveFirst(&(event->waitList))) != (tNode*) 0)
	{
		task = (tTask*)tNodeParent(node, tTask, linkNode);
		task->waitEvent = (tEvent*) 0;
		task->eventMsg = msg;
		task->waitEventResult = result;
		task->state &= TASK_CLEAR_MASK;
		// 如果任务还在延时队列中，将其唤醒
		if(task->delayTicks != 0)
		{
			tTimeTaskWake(task);
		}
		// 将任务加入到就绪队列中
		tTaskSchedRdy(task);
	}
	
	tTask_Exit_Critical(status);
	return task;
}

void tEventRemoveTask(tTask* task, void* msg, uint32_t result)
{
	uint32_t status = tTask_Enter_Critical();
	
	tListRemove(&(((tEvent *)task->waitEvent)->waitList), &(task->linkNode));
	task->waitEvent = (tEvent*) 0;
	task->eventMsg = msg;
	task->waitEventResult = result;
	task->state &= TASK_CLEAR_MASK;
	
	tTask_Exit_Critical(status);
}

uint32_t tEventRemoveAll(tEvent* event, void* msg, uint32_t result)
{
	uint32_t count = 0;
	tNode* node = (tNode*)0;
	tTask* task = (tTask*)0;
	
	uint32_t status = tTask_Enter_Critical();
	
	count = tListCount(&(event->waitList));
	while((node = tListRemoveFirst(&(event->waitList))) != (tNode*)0)
	{
		task = tNodeParent(node, tTask, linkNode);
		task->eventMsg = msg;
		task->waitEvent = (tEvent*)0;
		task->waitEventResult = result;
		task->state &= TASK_CLEAR_MASK;
		
		if(task->delayTicks > 0)
		{
			tTimeTaskWake(task);
		}
		tTaskSchedRdy(task);
	}
	
	tTask_Exit_Critical(status);
	
	return count;
}


uint32_t tEventWaitCount(tEvent* event)
{
	uint32_t count = 0;
	uint32_t status = tTask_Enter_Critical();
	count = tListCount(&(event->waitList));
	tTask_Exit_Critical(status);
	return count;
}
