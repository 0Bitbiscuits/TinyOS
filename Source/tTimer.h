#ifndef __TTIMER_H
#define __TTIMER_H

#include "tEvent.h"

#define TIMER_CONFIG_TYPE_HARD 			(1 << 0)
#define TIMER_CONFIG_TYPE_SOFT			(0 << 0)

typedef enum _tTimerState
{
	tTimerCreated,
	tTimerStarted,
	tTimerRunning,
	tTimerStopped,
	tTimerDestroyed,
}tTimerState;

#define TIMER_CONFIG_TYPE_HARD 			(1 << 0)
#define TIMER_CONFIG_TYPE_SOFT 			(0 << 0)

typedef struct _tTimer
{
	tNode linkNode;
	uint32_t startDelayTicks;
	uint32_t durationTicks;
	uint32_t delayTicks;
	void (*timerFunc) (void* arg);
	void *arg;
	uint32_t config;
	tTimerState state;
}tTimer;

typedef struct _tTimerInfo
{
	uint32_t startDelayTicks;
	uint32_t durationTicks;
	uint32_t delayTicks;
	uint32_t config;
	tTimerState state;
	void (*timerFunc) (void* arg);
	void *arg;
}tTimerInfo;

void tTimerInit(tTimer *timer, uint32_t delayTicks, uint32_t durationTicks,
	void (*timerFunc) (void *arg), void* arg, uint32_t config);
void tTimerModuleInit(void);
void tTimerInitTask(void);
void tTimerModuleTickNotify(void);
void tTimerStart(tTimer* timer);
void tTimerStop(tTimer* timer);
void tTimerDestroy(tTimer* timer);
void tTimerGetInfo(tTimer* timer, tTimerInfo* info);

#endif
