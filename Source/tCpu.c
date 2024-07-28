#include "tinyOS.h"
#include "ARMCM3.h"


// 系统时钟配置
void tTask_Systick_config(uint32_t ms)
{
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;				// 重载计数器
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);	// 优先级范围 -- 3bit
	SysTick->VAL = 0;												// 计数器
	// 系统时钟源	开启系统时钟中断	开启系统时钟
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk	|
					SysTick_CTRL_TICKINT_Msk 	|
					SysTick_CTRL_ENABLE_Msk		;
}



// 使用系统时钟中断进行调度
void SysTick_Handler(void)
{
	tTaskSystemTickHandler();
}
