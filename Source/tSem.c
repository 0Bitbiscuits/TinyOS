#include "tinyOS.h"

// 初始化信号量
void tSemInit(tSem* sem, uint32_t startCount, uint32_t maxCount)
{
	tEventInit(&(sem->event), tEventTypeSem);
	
	sem->maxCount = maxCount;
	if(maxCount == 0) // 说明计数值没有上限
	{
		sem->count = startCount;
	}
	else
	{
		sem->count = (startCount > maxCount)? maxCount : startCount;
	}
}

// 等待获取信号量
uint32_t tSemWait(tSem* sem, uint32_t waitTicks)
{
	uint32_t status = tTask_Enter_Critical();
	
	// 存在资源，资源数量减一
	if(sem->count > 0)
	{
		sem->count--;
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	else // 否则任务进入等待状态
	{
		// 将任务加入到等待队列
		tEventWait(&(sem->event), currentTask, (void*)0, tEventTypeSem, waitTicks);
		tTask_Exit_Critical(status);
		
		tTaskSched();
		
		return currentTask->waitEventResult;
	}
}


// 无等待获取信号量，返回结果为tError型
uint32_t tSemNoWaitGet(tSem* sem)
{
	uint32_t status = tTask_Enter_Critical();
	
	if(sem->count > 0)
	{
		sem->count--;
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	else
	{
		tTask_Exit_Critical(status);
		return tErrorResourceUnvaliable;
	}
}

// 唤醒等待队列或增加信号量
void tSemNotify(tSem* sem)
{
	uint32_t status = tTask_Enter_Critical();
	
	if(tEventWaitCount(&(sem->event)))
	{
		tTask* task = tEventWake(&(sem->event), (void*)0, tErrorNoError);
		if(task->prio < currentTask->prio) 
		{
			tTaskSched();
		}
	}
	else
	{
		++sem->count;
		if((sem->maxCount != 0) && (sem->count > sem->maxCount))
		{
			sem->count = sem->maxCount;
		}
	}
	tTask_Exit_Critical(status);
}


// 释放信号量
uint32_t tSemDestroy(tSem* sem)
{
	uint32_t count = 0;
	uint32_t status = tTask_Enter_Critical();
	
	count = tEventRemoveAll(&(sem->event), (void*)0, tErrorDelete);
	sem->count = 0;
	
	tTask_Exit_Critical(status);
	if(count) tTaskSched();
	return count;
}

void tSemGetInfo(tSem* sem, tSemInfo* info)
{
	uint32_t status = tTask_Enter_Critical();
	
	info->count = sem->count;
	info->maxCount = sem->maxCount;
	info->taskCount = tEventWaitCount(&(sem->event));
	
	tTask_Exit_Critical(status);
}
