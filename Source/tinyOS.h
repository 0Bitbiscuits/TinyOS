#ifndef __TINYOS_H
#define __TINYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tConfig.h"
#include "tEvent.h"
#include "tTask.h"
#include "tSem.h"
#include "tMailbox.h"
#include "tMemBlock.h"
#include "tFlagGroup.h"
#include "tMutex.h"
#include "tTimer.h"

typedef enum _ERROR_CODE
{
	tErrorNoError = 0,
	tErrorTimeout,
	tErrorResourceUnvaliable,
	tErrorDelete,
	tErrorResourceFull,
	tErrorOwner,
}tERROR;

// CPU
void tTaskRunFirst(void);
void tTaskSwitch(void);
// 时钟中断
void SysTick_Handler(void);
void tTaskSystemTickHandler(void);
// 延时
void tTaskDelay(uint32_t delay);
void delay(uint16_t count);
// 延时列表
void tTaskDelayInit(void);
void tTimeTaskWait(tTask* task, uint32_t ticks);
void tTimeTaskWake(tTask* task);
void tTimeKeep(void);
// 任务函数
void task1_handler(void* param);
void task2_handler(void* param);
void task3_handler(void* param);
void task4_handler(void* param);
void idleTask_handler(void* param);
// CPU使用率
void initCpuUsageStat(void);
void cpuUsageSyncWithSysTick(void);
void checkCpuUsage(void);
double tCpugetUsage(void);
void ticktop(void);

#endif
