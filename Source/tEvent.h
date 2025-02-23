#ifndef __TEVENT_H
#define __TEVENT_H

#include "tLib.h"
#include "tTask.h"


typedef enum _tEventType
{
	tEventTypeUnknow = 1 << 16,
	tEventTypeSem = 1 << 17,
	tEventTypeMbox = 1 << 18,
	tEventTypeMemBlock = 1 << 19,
	tEventTypeFlagGroup = 1 << 20,
	tEventTypeMutex = 1 << 21,
}tEventType;

typedef struct _tEvent
{
	tEventType type;
	tList waitList;
}tEvent;


void tEventInit(tEvent* event, tEventType type);
void tEventWait(tEvent* event, tTask* task, void* msg, uint32_t state, uint32_t timeout);
tTask* tEventWake(tEvent* event, void* msg, uint32_t result);
void tEventRemoveTask(tTask* task, void* msg, uint32_t result);
uint32_t tEventRemoveAll(tEvent* event, void* msg, uint32_t result);
uint32_t tEventWaitCount(tEvent* event);


#endif
