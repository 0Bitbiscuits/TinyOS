#include "tinyOS.h"

// 延时队列
tList Delay_List;

// 延时函数
void delay(uint16_t count)
{
	while(--count);
}

// 带调度的延时函数
void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTask_Enter_Critical();
	tTimeTaskWait(currentTask, delay);
	tTaskSchedUnRdy(currentTask);
	tTask_Exit_Critical(status);
	tTaskSched();
}

// 延时队列初始化
void tTaskDelayInit(void)
{
	tListInit(&Delay_List);
}

// 延时队列的插入
void tTimeTaskWait(tTask* task, uint32_t ticks)
{
	uint32_t status = tTask_Enter_Critical();
	task->delayTicks = ticks;
	task->state |= TASK_DELAY;
	tListAddLast(&Delay_List, &(task->delayNode));
	tTask_Exit_Critical(status);
}

// 延时队列的删除
void tTimeTaskWake(tTask* task)
{
	uint32_t status = tTask_Enter_Critical();
	task->state &= ~(uint32_t)(TASK_DELAY);
	tListRemove(&Delay_List, &(task->delayNode));
	tTask_Exit_Critical(status);
}


// 遍历延时列表
void tTimeKeep(void)
{
	tNode* node = (tNode*)0;
	for(node = Delay_List.headNode.nextNode; node != &(Delay_List.headNode); node = node->nextNode)
	{
		tTask* task = tNodeParent(node, tTask, delayNode);
		if(--(task->delayTicks) == 0)
		{
			if(task->waitEvent) 
				tEventRemoveTask(task, (void*)0, tErrorTimeout);
			
			tTimeTaskWake(task);
			tTaskSchedRdy(task);
		}
	}
}

// 任务在延时队列中时进行删除
void tTimeTaskRemove(tTask* task)
{
	tListRemove(&Delay_List, &(task->delayNode));
}
