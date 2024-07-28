#include "tinyOS.h"
#include "tLib.h"
#include "ARMCM3.h"
#include "tConfig.h"

tTask idleTask;
tTaskStack idleEnv[STACK_SIZE]; // 空闲任务栈

// 主函数
int main()
{
	// 初始化调度资源
	tTaskSchedInit();
	tTaskDelayInit();
	tTimerModuleInit();
	initCpuUsageStat();
	// 空闲任务
	tTaskInit(&idleTask, idleTask_handler, (void*)0x33333333, TINYOS_PRIO_COUNT - 1, 10, idleEnv, sizeof(idleEnv));

	// 将最高优先级的调度队列加入到调度中
	nextTask = tTask_HighPrio_Ready();
	
	// 开始调度
	tTaskRunFirst();
	return 0;
}
