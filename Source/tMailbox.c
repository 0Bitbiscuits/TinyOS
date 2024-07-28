#include "tinyOS.h"


void tMboxInit(tMbox* mbox, void**msgBuffer, uint32_t maxCount)
{
	tEventInit(&(mbox->event), tEventTypeMbox);
	
	mbox->msgBuffer = msgBuffer;
	mbox->count = 0;
	mbox->maxCount = maxCount;
	mbox->read_offset = 0;
	mbox->write_offset = 0;
}

uint32_t tMboxWait(tMbox* mbox, void** msg, uint32_t waitTicks)
{
	uint32_t status = tTask_Enter_Critical();
	
	if(mbox->count > 0)
	{
		--mbox->count;
		*msg = mbox->msgBuffer[mbox->read_offset];
		mbox->read_offset = (mbox->read_offset + 1) % mbox->maxCount;
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	else
	{
		tEventWait(&(mbox->event), currentTask, (void*) 0, tEventTypeMbox, waitTicks);
		tTask_Exit_Critical(status);
		tTaskSched();
		*msg = currentTask->eventMsg;
		return currentTask->waitEventResult;
	}
}

uint32_t tMboxNoWaitGet(tMbox* mbox, void** msg)
{
	uint32_t status = tTask_Enter_Critical();
	if(mbox->count > 0)
	{
		--mbox->count;
		*msg = mbox->msgBuffer[mbox->read_offset];
		mbox->read_offset = (mbox->read_offset + 1) % mbox->maxCount;
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	tTask_Exit_Critical(status);
	return tErrorResourceUnvaliable;
}

uint32_t tMboxNotify(tMbox* mbox, void* msg, uint32_t notifyOption)
{
	uint32_t status = tTask_Enter_Critical();
	tTask* task;
	// 检查有没有等待资源的任务
	if(tEventWaitCount(&(mbox->event))) // 存在等待的任务
	{
		task = tEventWake(&(mbox->event), msg, tErrorNoError);
		// 如果任务优先级更高那么执行调度
		if(task->prio > currentTask->prio) tTaskSched();
	}
	else
	{
		// 不存在等待的任务
		// 判断缓存区是否已满
		if(mbox->count >= mbox->maxCount)
		{
			tTask_Exit_Critical(status);
			return tErrorResourceFull;
		}
		if(notifyOption) // 高优先消息
		{
			mbox->read_offset = (mbox->read_offset - 1 + mbox->maxCount) % mbox->maxCount;
			mbox->msgBuffer[mbox->read_offset] = msg;
		}
		else
		{
			mbox->msgBuffer[mbox->write_offset++] = msg;
			mbox->write_offset %= mbox->maxCount;
		}
		mbox->count++;
	}
	tTask_Exit_Critical(status);
	return tErrorNoError;
}


// 清空邮箱
void tMboxFlush(tMbox* mbox)
{
	uint32_t status = tTask_Enter_Critical();
	if(tEventWaitCount(&(mbox->event)) == 0) // 没有任务等待说明，邮箱中存在数据
	{
		mbox->read_offset = 0;
		mbox->write_offset = 0;
		mbox->count = 0;
	}
	tTask_Exit_Critical(status);
}

// 删除邮箱
uint32_t tMboxDestory(tMbox* mbox)
{
	uint32_t status = tTask_Enter_Critical();
	//tMboxFlush(mbox);
	uint32_t count = tEventRemoveAll(&(mbox->event), (void*)0, tErrorDelete);
	tTask_Exit_Critical(status);
	if(count > 0) tTaskSched();
	return count;
}

void tMboxGetInfo(tMbox* mbox, tMboxInfo* info)
{
	uint32_t status = tTask_Enter_Critical();
	
	info->count = mbox->count;
	info->maxCount = mbox->maxCount;
	info->read_offset = mbox->read_offset;
	info->write_offset = mbox->write_offset;
	
	tTask_Exit_Critical(status);
}
