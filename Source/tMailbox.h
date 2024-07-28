#ifndef __TMAILBOX_H
#define __TMAILBOX_H

#include "tEvent.h"

typedef struct _tMbox
{
	tEvent event;
	uint32_t count;
	uint32_t maxCount;
	uint32_t read_offset;
	uint32_t write_offset;
	void** msgBuffer;
}tMbox;

typedef struct _tMboxInfo
{
	uint32_t count;
	uint32_t maxCount;
	uint32_t read_offset;
	uint32_t write_offset;
}tMboxInfo;

void tMboxInit(tMbox* mbox, void**msgBuffer, uint32_t maxCount);
uint32_t tMboxWait(tMbox* mbox, void** msg, uint32_t waitTicks);
uint32_t tMboxNoWaitGet(tMbox* mbox, void** msg);
uint32_t tMboxNotify(tMbox* mbox, void* msg, uint32_t notifyOption);
void tMboxFlush(tMbox* mbox);
uint32_t tMboxDestory(tMbox* mbox);
void tMboxGetInfo(tMbox* mbox, tMboxInfo* info);

#endif
