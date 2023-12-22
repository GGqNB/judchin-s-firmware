#include <stm32f0xx.h>
static __IO uint32_t TimingDelay;

void SysTick_conf (__IO uint32_t TimerTick); 
void Delay_ms(__IO uint32_t nTime);
void init_TIM14 (void);
void TIM14_IRQHandler(void);
