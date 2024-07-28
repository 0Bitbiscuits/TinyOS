#ifndef __TMUTEX_H
#define __TMUTEX_H

#include "tEvent.h"

typedef struct _tMutex
{
	tEvent event;					// 事件
	uint32_t lockedCount;			// 锁定次数
	tTask *owner;					// 锁的拥有者
	uint32_t ownerOriginPrio;		// 锁拥有者的原始优先级
}tMutex;

typedef struct _tMutexInfo
{
	uint32_t taskCount;
	uint32_t ownerPrio;
	uint32_t inheritedPrio;
	tTask* owner;
	uint32_t lockedCount;
}tMutexInfo;

void tMutexInit(tMutex* mutex);
uint32_t tMutexWait(tMutex *mutex, uint32_t waitTicks);
uint32_t tMutexNoWaitGet(tMutex* mutex);
uint32_t tMutexNotify(tMutex* mutex);
uint32_t tMutexDestroy(tMutex* mutex);
void tMutexGetInfo(tMutex* mutex, tMutexInfo* info);

#endif
