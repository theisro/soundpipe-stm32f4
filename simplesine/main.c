#include "main.h"

RCC_ClocksTypeDef RCC_Clocks;
extern __IO uint8_t LED_Toggle;

int main(void)
{ 
  /* Initialize LEDS */
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED6);
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);
 
  /* Green Led On: start of application */
       
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 10);
  
  WavePlayBack(I2S_AudioFreq_22k); 
  while (1);
  
}
