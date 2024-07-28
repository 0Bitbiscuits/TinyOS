#include "tinyOS.h"

void tMutexInit(tMutex* mutex)
{
	tEventInit(&(mutex->event), tEventTypeMutex);
	mutex->lockedCount = 0;
	mutex->owner = (tTask*)0;
	mutex->ownerOriginPrio = TINYOS_PRIO_COUNT;
}


uint32_t tMutexWait(tMutex *mutex, uint32_t waitTicks)
{
	uint32_t status = tTask_Enter_Critical();
	
	// 当互斥量空闲时
	if(mutex->lockedCount <= 0) 
	{
		mutex->owner = currentTask;
		mutex->ownerOriginPrio = currentTask->prio;
		mutex->lockedCount++;
		
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	else // 互斥量被占有时
	{
		if(mutex->owner == currentTask)
		{
			mutex->lockedCount++;
			tTask_Exit_Critical(status);
			return tErrorNoError;
		}
		else
		{
			// 当请求资源的任务的优先级比资源拥有者高时
			if(currentTask->prio < mutex->owner->prio)
			{
				tTask* owner = mutex->owner;
				// 提高资源拥有者的优先级
				if(owner->state == TASK_READY)
				{
					// 当任务处于就绪队列中时需要重置优先级位图
					tTaskSchedUnRdy(owner);
					owner->prio = currentTask->prio;
					tTaskSchedRdy(owner);
				}
				else
				{
					owner->prio = currentTask->prio;
				}
			}
			
			tEventWait(&(mutex->event), currentTask, (void*)0, tEventTypeMutex, waitTicks);
			tTask_Exit_Critical(status);
			tTaskSched();
			return currentTask->waitEventResult;
		}
	}
}


uint32_t tMutexNoWaitGet(tMutex* mutex)
{
	uint32_t status = tTask_Enter_Critical();
	
	// 当互斥量空闲时
	if(mutex->lockedCount <= 0) 
	{
		mutex->owner = currentTask;
		mutex->ownerOriginPrio = currentTask->prio;
		mutex->lockedCount++;
	}
	else
	{
		if(mutex->owner == currentTask)
		{
			mutex->lockedCount++;
		}
		else
		{
			tTask_Exit_Critical(status);
			return tErrorResourceUnvaliable;
		}
	}
	
	tTask_Exit_Critical(status);
	return tErrorNoError;
}

uint32_t tMutexNotify(tMutex* mutex)
{
	uint32_t status = tTask_Enter_Critical();
	
	// 互斥量空闲则直接退出
	if(mutex->lockedCount <= 0)
	{
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	
	// 唤醒互斥量的任务不是拥有者，直接退出
	if(mutex->owner != currentTask)
	{
		tTask_Exit_Critical(status);
		return tErrorOwner;
	}
	
	// 拥有者唤醒信号量则信号量直接减一
	if(--mutex->lockedCount > 0)
	{
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	
	// 信号量为0时
	// 执行了优先级继承，那么在唤醒时就要先恢复任务优先级
	if(mutex->ownerOriginPrio != mutex->owner->prio)
	{
		if(mutex->owner->state == TASK_READY)
		{
			// 在对任务的优先级赋值之前要先将任务从就绪队列中删除并重置优先级位图
			tTaskSchedUnRdy(mutex->owner);
			currentTask->prio = mutex->ownerOriginPrio;
			tTaskSchedRdy(mutex->owner);
		}
		else
		{
			// 任务不在就绪队列中直接赋值就可以
			currentTask->prio = mutex->ownerOriginPrio;
		}
	}
	
	// 当资源可用时，随机唤醒一个等待队列中的任务
	if(tEventWaitCount(&(mutex->event)) > 0)
	{
		tTask* task = tEventWake(&(mutex->event), (void*)0, tErrorNoError);
		
		mutex->owner = task;
		mutex->ownerOriginPrio = task->prio;
		mutex->lockedCount++;
		
		if(task->prio > currentTask->prio)
		{
			tTaskSched();
		}
	}
	tTask_Exit_Critical(status);
	return tErrorNoError;
}


// 删除信号量
uint32_t tMutexDestroy(tMutex* mutex)
{
	uint32_t count = 0;
	uint32_t status = tTask_Enter_Critical();
	
	if(mutex->lockedCount > 0)
	{
		// 发生优先级继承
		if(mutex->ownerOriginPrio != mutex->owner->prio)
		{
			if(mutex->owner->state == TASK_READY)
			{
				tTaskSchedUnRdy(mutex->owner);
				mutex->owner->prio = mutex->ownerOriginPrio;
				tTaskSchedRdy(mutex->owner);
			}
			else
			{
				mutex->owner->prio = mutex->ownerOriginPrio;
			}
		}
		
		count = tEventRemoveAll(&(mutex->event), (void*)0, tErrorDelete);
		if(count) tTaskSched();
	}
	tTask_Exit_Critical(status);
	
	return count;
}


// 获取信号量信息
void tMutexGetInfo(tMutex* mutex, tMutexInfo* info)
{
	uint32_t status = tTask_Enter_Critical();
	
	info->taskCount = tEventWaitCount(&(mutex->event));
	info->ownerPrio = mutex->ownerOriginPrio;
	if(mutex->owner != (tTask*)0)
		info->inheritedPrio = mutex->owner->prio;
	else
		info->inheritedPrio = TINYOS_PRIO_COUNT;
	info->owner = mutex->owner;
	info->lockedCount = mutex->lockedCount;
	
	tTask_Exit_Critical(status);
}

