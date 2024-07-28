#include "tinyOS.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL 			0xE000ED04
#define NVIC_PENDSVSET 			0x10000000
#define NVIC_SYSPRI2 			0xE000ED22
#define NVIC_PENDSV_PRI			0x000000FF

#define MEM32(addr) 			*(volatile unsigned long *) (addr)
#define MEM8(addr)				*(volatile unsigned char *) (addr)
	

__asm void PendSV_Handler(void)
{
	IMPORT currentTask
	IMPORT nextTask
	
	MRS R0, PSP
	CBZ R0, PendSVHandler_nosave
	STMDB R0!, {R4-R11}
	LDR R1, =currentTask
	LDR R1, [R1]
	STR R0, [R1]
	NOP
	
PendSVHandler_nosave
	LDR R0, =currentTask
	LDR R1, =nextTask
	LDR R2, [R1]
	STR R2, [R0]
	
	LDR R0, [R2]
	
	LDMIA R0!, {R4-R11}
	
	
	MSR PSP, R0
	ORR LR, LR, #0x04
	
	BX LR
}


void tTaskRunFirst()
{
	// 初始化用户栈的栈顶
	__set_PSP(0);
	// 设置PENDSV的优先级
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	// 触发PENDSV
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void tTaskSwitch(void)
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}
