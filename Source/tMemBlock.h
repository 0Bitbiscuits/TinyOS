#ifndef __TMEMBLOCK_H
#define __TMEMBLOCK_H

#include "tEvent.h"

typedef struct _tMemBlock
{
	tEvent event;			// 维护等待事件
	void* memStart;			// 存储块的起始地址
	uint32_t blockSize;		// 存储块大小
	uint32_t maxCount;		// 存储块总数量
	tList blockList;		// 维护存储块列表
}tMemBlock;

typedef struct _tMemBlockInfo
{
	uint32_t block_size;
	uint32_t maxCount;
	uint32_t blockCount;
	uint32_t taskCount;
}tMemBlockInfo;

void tMemBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t tMemBlockWait(tMemBlock* memBlock, uint8_t **mem, uint32_t waitTicks);
uint32_t tMemBlockNoWaitGet(tMemBlock* memBlock, uint8_t **mem);
uint32_t tMemBlockNotify(tMemBlock* memBlock, uint8_t* mem);
uint32_t tMemBlockDestroy(tMemBlock* memBlock);
void tMemBlockInfoGet(tMemBlock* memBlock, tMemBlockInfo* info);

#endif
