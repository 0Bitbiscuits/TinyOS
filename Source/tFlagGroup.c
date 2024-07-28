#include "tinyOS.h"


// 事件标志组初始化
void tFlagGroupInit(tFlagGroup* flagGroup, uint32_t flags)
{
	tEventInit(&(flagGroup->event), tEventTypeFlagGroup);
	flagGroup->flag = flags;
}

// 检查并消耗事件标志
// flags -- 事件标志
static uint32_t tFlagGroupCheckAndConsume(tFlagGroup* flagGroup, uint32_t type, uint32_t *flags)
{
	uint32_t srcFlag = *flags;
	uint32_t isSet = type & TFLAGGROUP_SET;
	uint32_t isALL = type & TFLAGGROUP_ALL;
	uint32_t isConsume = type & TFLAGGROUP_CONSUME;
	
	// 将新事件标志和组内事件进行匹配
	// isSet为1则匹配1，为0则匹配0，看存在那些位是相同的
	// calFlag 触发事件和组内事件的相同位
	uint32_t calFlag = isSet ? (flagGroup->flag & srcFlag) : (~flagGroup->flag & srcFlag);
	// 全部匹配
	if(((isALL != 0) && (calFlag == srcFlag)) || ((isALL == 0) && (calFlag != 0))) // 匹配成功
	{
		if(isConsume)
		{
			if(isSet)	// 把标志位中的1消耗掉
				flagGroup->flag &= ~srcFlag; 
			else 
				flagGroup->flag |= srcFlag;
		}
		*flags = calFlag;
		return tErrorNoError;
	}
	*flags = calFlag; // 将可置位的位传回
	return tErrorResourceUnvaliable;
}


// 设置任务等待的信号
uint32_t tFlagGroupWait(tFlagGroup* flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag, uint32_t waitTicks)
{
	uint32_t result;
	uint32_t flags = requestFlag;
	uint32_t status = tTask_Enter_Critical();
	
	// 检查任务等待的信号和事件组接收的信号是否相同
	result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
	// 不同的话，设置对应的标记变量并将任务挂起
	if(result != tErrorNoError)
	{
		// 给标记赋值
		currentTask->waitFlagsType = waitType;
		currentTask->eventFlags = requestFlag;
		tEventWait(&(flagGroup->event), currentTask, (void*)0, tEventTypeFlagGroup, waitTicks);
		
		tTask_Exit_Critical(status);
		
		tTaskSched();
		
		*resultFlag = currentTask->eventFlags;
		result = currentTask->waitEventResult;
	}
	else
	{
		*resultFlag = flags;
		tTask_Exit_Critical(status);
	}
	return result;
}
uint32_t tFlagGroupNoWaitGet(tFlagGroup* flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag)
{
	uint32_t result;
	uint32_t flags = requestFlag;
	uint32_t status = tTask_Enter_Critical();
	
	// 检查任务等待的信号和事件组接收的信号是否相同
	result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
	
	*resultFlag = flags;
	tTask_Exit_Critical(status);
	
	return result;
}
void tFlagGroupNotify(tFlagGroup* flagGroup, uint8_t isSet, uint32_t flag)
{
	tList* waitList = (tList*)0;
	tNode *node = (tNode*)0, *nextNode = (tNode*)0;
	uint32_t status = tTask_Enter_Critical();
	
	if(isSet)
	{
		flagGroup->flag |= flag;
	}
	else
	{
		flagGroup->flag &= ~flag;
	}
	
	uint32_t result = 0;
	uint32_t relSuccess = 0;
	waitList = &flagGroup->event.waitList;
	for(node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
	{
		tTask *task = tNodeParent(node, tTask, linkNode);
		uint32_t flags = task->eventFlags;
		nextNode = node->nextNode;
		
		result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
		if(result == tErrorNoError)
		{
			task->eventFlags = flags;
			tEventWake(task->waitEvent, (void*)0, tErrorNoError);
			relSuccess = 1;
		}
	}
	if(relSuccess) tTaskSched();
	
	tTask_Exit_Critical(status);
}

void tFlagGroupGetInfo(tFlagGroup* flagGroup, tFlagGroupInfo* info)
{
	uint32_t status = tTask_Enter_Critical();
	info->flags = flagGroup->flag;
	info->taskCount = tEventWaitCount(&(flagGroup->event));
	tTask_Exit_Critical(status);
}
uint32_t tFlagGroupDestroy(tFlagGroup* flagGroup)
{
	uint32_t status = tTask_Enter_Critical();
	uint32_t count = tEventRemoveAll(&(flagGroup->event), (void*)0, tErrorDelete);
	tTask_Exit_Critical(status);
	if(count) tTaskSched();
	return count;
}
