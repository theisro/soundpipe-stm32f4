#include "main.h"

RCC_ClocksTypeDef RCC_Clocks;
extern __IO uint8_t LED_Toggle;

void adc_configure(){
 ADC_InitTypeDef ADC_init_structure; //Structure for adc confguration
 GPIO_InitTypeDef GPIO_initStructre; //Structure for analog input pin
 //Clock configuration
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//The ADC1 is connected the APB2 peripheral bus thus we will use its clock source
 RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOCEN,ENABLE);//Clock for the ADC port!! Do not forget about this one ;)
 //Analog pin configuration
 GPIO_initStructre.GPIO_Pin = GPIO_Pin_0;//The channel 10 is connected to PC0
 GPIO_initStructre.GPIO_Mode = GPIO_Mode_AN; //The PC0 pin is configured in analog mode
 GPIO_initStructre.GPIO_PuPd = GPIO_PuPd_NOPULL; //We don't need any pull up or pull down
 GPIO_Init(GPIOC,&GPIO_initStructre);//Affecting the port with the initialization structure configuration
 //ADC structure configuration
 ADC_DeInit();
 ADC_init_structure.ADC_DataAlign = ADC_DataAlign_Right;//data converted will be shifted to right
 ADC_init_structure.ADC_Resolution = ADC_Resolution_12b;//Input voltage is converted into a 12bit number giving a maximum value of 4096
 ADC_init_structure.ADC_ContinuousConvMode = ENABLE; //the conversion is continuous, the input data is converted more than once
 ADC_init_structure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;// conversion is synchronous with TIM1 and CC1 (actually I'm not sure about this one :/)
 ADC_init_structure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//no trigger for conversion
 ADC_init_structure.ADC_NbrOfConversion = 1;//I think this one is clear :p
 ADC_init_structure.ADC_ScanConvMode = DISABLE;//The scan is configured in one channel
 ADC_Init(ADC1,&ADC_init_structure);//Initialize ADC with the previous configuration
 //Enable ADC conversion
 ADC_Cmd(ADC1,ENABLE);
 //Select the channel to be read from
 ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_144Cycles);
}

int main(void)
{ 
  /* Initialize LEDS */
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED6);
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);
 
  adc_configure();
  /* Green Led On: start of application */
       
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);
  
  WavePlayBack(I2S_AudioFreq_22k); 
  while (1);
}
