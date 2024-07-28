#include "tinyOS.h"
#include "string.h"

#define TICKS_PER_SEC					(1000 / TINYOS_SYSTICK_MS)

void tInitApp(void);

// 任务结构体
tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;


// 任务运行栈空间
tTaskStack task1Env[STACK_SIZE];
tTaskStack task2Env[STACK_SIZE];
tTaskStack task3Env[STACK_SIZE];
tTaskStack task4Env[STACK_SIZE];

void timerFunc(void* arg)
{
	uint32_t * ptrBit = (uint32_t *)arg;
	*ptrBit ^= 0x1;
}

/**
    \brief        	任务运行的函数实体
    \param[in]  	函数的形参
      
    \param[out] none
    \retval     none
*/
int task1Flag = 0;
double usage = 0.0f;
void task1_handler(void* param)
{	
	while(1)
	{
		task1Flag = 0;
		delay(100);
		task1Flag = 1;
		//tTaskDelay(1);
		delay(100);
		usage = tCpugetUsage();
	}
}
int task2Flag = 0;
uint32_t value = 0;
void task2_handler(void* param)
{
	while(1)
	{
		task2Flag = 0;
//		tTaskDelay(1);
		delay(100);
		task2Flag = 1;
//		tTaskDelay(1);
		delay(100);
	}
}
int task3Flag = 0;
void task3_handler(void* param)
{
	uint8_t cnt = 0;
	while(++cnt <= 100)
	{
		task3Flag = 0;
//		tTaskDelay(1);
		delay(100);
		task3Flag = 1;
//		tTaskDelay(1);
		delay(100);
	}
}
int task4Flag = 0;
void task4_handler(void* param)
{
	uint8_t cnt = 0;
	while(++cnt <= 100)
	{
		task4Flag = 0;
		tTaskDelay(1);
		task4Flag = 1;
		tTaskDelay(1);
	}
	return;
}
static uint32_t enableCpuUsageState;
static double cpuUsage;
uint32_t idleCount = 0;
uint32_t idleMaxCount;
uint32_t tickCount = 0;
// 初始化变量
void initCpuUsageStat(void)
{
	idleCount = 0;
	cpuUsage = 0.0f;
	tickCount = 0;
	idleMaxCount = 0;
	enableCpuUsageState = 0;
}
// 等待同步
void cpuUsageSyncWithSysTick(void)
{
	while(enableCpuUsageState == 0);
}
// 检查时钟滴答计数器，到达一秒后计算cpu使用率
void checkCpuUsage(void)
{
	if(enableCpuUsageState == 0)
	{
		enableCpuUsageState = 1;
		return;
	}
	if(tickCount == TICKS_PER_SEC)
	{
		idleMaxCount = idleCount;
		idleCount = 0;
		
		tTask_Sched_Enable();
	}
	else if(tickCount % TICKS_PER_SEC == 0)
	{
		cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
		idleCount = 0;
	}
}
// 增加时钟计数器
void ticktop()
{
	tickCount++;
}
void idleTask_handler(void* param)
{
	// 关闭调度
	tTask_Sched_Disable();
	// 将任务加入调度
	tInitApp();
//	// 启动软件定时器
//	tTimerInitTask();
	// 设置定时调度
	tTask_Systick_config(TINYOS_SYSTICK_MS);
	// 同步时钟计数器
	cpuUsageSyncWithSysTick();
	while(1)
	{
		uint32_t status = tTask_Enter_Critical();
		idleCount++;
		tTask_Exit_Critical(status);
	}
}

double tCpugetUsage(void)
{
	double usage = 0.0f;
	uint32_t status = tTask_Enter_Critical();
	usage = cpuUsage;
	tTask_Exit_Critical(status);
	return usage;
}

void tInitApp(void)
{
	// 初始化任务资源
	tTaskInit(&tTask1, task1_handler, (void*)0x11111111, 0, 10, task1Env, sizeof(task1Env));
	tTaskInit(&tTask2, task2_handler, (void*)0x22222222, 1, 10, task2Env, sizeof(task2Env));
	tTaskInit(&tTask3, task3_handler, (void*)0x33333333, 0, 10, task3Env, sizeof(task3Env));
	tTaskInit(&tTask4, task4_handler, (void*)0x44444444, 1, 10, task4Env, sizeof(task4Env));
}