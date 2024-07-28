#include "tinyOS.h"

void tMemBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCnt)
{
	uint8_t* memBlockStart = (uint8_t*) memStart; // 存储块的起始地址
	uint8_t* memBlockEnd = memBlockStart + blockSize* blockCnt;	// 存储块的末尾地址
	
	// 因为存储块需要由tNode进行连接，因此存储块的内存空间需要大于tNode所需要的空间
	if(blockSize< sizeof(tNode)) return; 
	
	// 给管理存储块的结构体进行初始化
	tEventInit(&(memBlock->event), tEventTypeMemBlock);
	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockCnt;
	
	// 初始化存储块链表
	tListInit(&memBlock->blockList);
	while(memBlockStart < memBlockEnd)
	{
		tNodeInit((tNode*)memBlockStart);
		tListAddLast(&(memBlock->blockList), (tNode*)memBlockStart);
		memBlockStart += blockSize; 
	}
	
}

uint32_t tMemBlockWait(tMemBlock* memBlock, uint8_t **mem, uint32_t waitTicks)
{
	uint32_t status = tTask_Enter_Critical();
	
	if(tListCount(&(memBlock->blockList)) > 0) // 如果存在空闲存储块则直接分配
	{
		*mem = (uint8_t*)tListRemoveFirst(&(memBlock->blockList));
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	else
	{
		tEventWait(&(memBlock->event), currentTask, (void*)0, tEventTypeMemBlock, waitTicks);
		tTask_Exit_Critical(status);
		
		tTaskSched();
		
		*mem = currentTask->eventMsg;
		return currentTask->waitEventResult;
	}
}

uint32_t tMemBlockNoWaitGet(tMemBlock* memBlock, uint8_t **mem)
{
	uint32_t status = tTask_Enter_Critical();
	
	if(tListCount(&(memBlock->blockList)) > 0) // 如果存在空闲存储块则直接分配
	{
		*mem = (uint8_t*)tListRemoveFirst(&(memBlock->blockList));
		tTask_Exit_Critical(status);
		return tErrorNoError;
	}
	else
	{
		tTask_Exit_Critical(status);
		return tErrorResourceUnvaliable;
	}
}

uint32_t tMemBlockNotify(tMemBlock* memBlock, uint8_t* mem)
{
	tTask* task = (tTask*)0;
	uint32_t status = tTask_Enter_Critical();
	
	// 存在任务在等待资源
	if(tEventWaitCount(&(memBlock->event)) > 0) 
	{
		task = tEventWake(&(memBlock->event), mem, tErrorNoError);
		if(task->prio > currentTask->prio) tTaskSched();
	}
	else // 没有等待资源的任务
	{
		// 如果储存块已满
		if(tListCount(&(memBlock->blockList)) == memBlock->maxCount)
		{
			tTask_Exit_Critical(status);
			return tErrorResourceFull;
		}
		
		tListAddLast(&(memBlock->blockList), (tNode*)mem);
	}
	
	tTask_Exit_Critical(status);
	return tErrorNoError;
}

uint32_t tMemBlockDestroy(tMemBlock* memBlock)
{
	uint32_t status = tTask_Enter_Critical();
	uint32_t count = tEventRemoveAll(&(memBlock->event), (void*)0, tErrorDelete);
	tTask_Exit_Critical(status);
	if(count) tTaskSched();
	return count;
}

void tMemBlockInfoGet(tMemBlock* memBlock, tMemBlockInfo* info)
{
	uint32_t status = tTask_Enter_Critical();
	
	info->blockCount = tListCount(&(memBlock->blockList));
	info->taskCount = tEventWaitCount(&(memBlock->event));
	info->block_size = memBlock->blockSize;
	info->maxCount = memBlock->maxCount;
	
	tTask_Exit_Critical(status);
}
