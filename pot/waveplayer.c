#include "main.h"
#include "soundpipe.h"

#define MY_BUFSIZE 64

static volatile uint8_t nextbuf;
static int16_t buf0[MY_BUFSIZE];
static int16_t buf1[MY_BUFSIZE];

/* SOUNDPIPE */

#define NOSCS 5
static const uint8_t scale[] = {48, 60, 65, 67, 72, 79, 64, 52};

//static const uint8_t scale[] = {48, 62, 65, 67, 72, 79, 64, 50};

static uint8_t please_play = 0;
static sp_data *sp;
static sp_ftbl *ft;
static sp_osc *osc;
static sp_revsc *revsc;

volatile uint8_t state = 0;

uint32_t compute_buffer(int16_t *pbuf, int  bufsize);
uint32_t compute_drip(int16_t *pbuf, int  bufsize);
uint32_t compute_pluck(int16_t *pbuf, int  bufsize);

__IO uint32_t XferCplt = 0;

void ms_delay(int ms)
{
   while (ms-- > 0) {
      volatile int x=5971;
      while (x-- > 0)
         __asm("nop");
   }
}

int adc_convert(){
	ADC_SoftwareStartConv(ADC1);//Start the conversion
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));//Processing the conversion
	return ADC_GetConversionValue(ADC1); //Return the converted data
}

void WavePlayBack(uint32_t AudioFreq)
{ 
    /* note: these mallocs are NOT freed at the moment */
    sp_create(&sp);
    sp->sr = AudioFreq;
    sp->len = AudioFreq * 5;

    sp_ftbl_create(sp, &ft, 8192);
    sp_gen_sine(sp, ft);

    sp_osc_create(&osc);

    sp_osc_init(sp, osc, ft, 0);
    osc->freq = 500;
    osc->amp = 0.8f;
    uint32_t i;

    for(i = 0; i < MY_BUFSIZE; i++) {
        buf0[i] = 0;
        buf1[i] = 0;
    }

    sp_revsc_create(&revsc);
    sp_revsc_init(sp, revsc);

    /* Initialize wave player (Codec, DMA, I2C) */
    WavePlayerInit(AudioFreq);
    nextbuf = 1;

    compute_buffer(buf1, MY_BUFSIZE);
    Audio_MAL_Play((uint32_t)buf0, MY_BUFSIZE / 2);

    EVAL_AUDIO_Mute(AUDIO_MUTE_ON);
    while(1) {
        if (STM_EVAL_PBGetState(BUTTON_USER)) {
            ms_delay(500);  //bounce key
            state += 1;
            if (state > 4) {
                state = 0;
            }
        }

        switch(state) {
            case 0:
                STM_EVAL_LEDOff(LED3);
                STM_EVAL_LEDOn(LED4);
                STM_EVAL_LEDOff(LED5);
                STM_EVAL_LEDOff(LED6);
                break;
            case 1:
                STM_EVAL_LEDOn(LED3);
                STM_EVAL_LEDOff(LED4);
                STM_EVAL_LEDOff(LED5);
                STM_EVAL_LEDOff(LED6);
                break;
            case 2:
                STM_EVAL_LEDOff(LED3);
                STM_EVAL_LEDOff(LED4);
                STM_EVAL_LEDOn(LED5);
                STM_EVAL_LEDOff(LED6);
                break;
            case 3:
                STM_EVAL_LEDOff(LED3);
                STM_EVAL_LEDOff(LED4);
                STM_EVAL_LEDOff(LED5);
                STM_EVAL_LEDOn(LED6);
                break;
        }
     
        while(nextbuf == 1);
        if(please_play == 0) {
            EVAL_AUDIO_Mute(AUDIO_MUTE_OFF);
            please_play = 1;
        }
        compute_buffer(buf0, MY_BUFSIZE);
        while(nextbuf == 0);
        compute_buffer(buf1, MY_BUFSIZE);
    };

}
int WavePlayerInit(uint32_t AudioFreq)
{ 
    /* Initialize I2S interface */  
    EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);
    /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */  
    EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, 60, AudioFreq );

    return 0;
}

uint32_t compute_buffer(int16_t *pbuf, int bufsize) 
{
    int i, o;
    SPFLOAT fm = 0, lfo = 0;
    SPFLOAT tmp = 0;
    SPFLOAT r0, r1;
    for(i = 0; i < bufsize / 2; i+=2) {
        //sp_osc_compute(sp, osc, NULL, &lfo);
        fm = 0;
        tmp = 0;
        //fosc->indx = ((1.0 + lfo) * 0.5) * 6;
        //sp_fosc_compute(sp, fosc, NULL, &tmp);
		osc->freq = adc_convert();
        sp_osc_compute(sp, osc, NULL, &tmp);
        //fm += tmp;
        sp_revsc_compute(sp, revsc, &tmp, &tmp, &r0, &r1);
        pbuf[i] = (fm * 0.8 + r0 * 0.1) * 32767;
        pbuf[i + 1] = (fm * 0.8  + r1 * 0.1) * 32767;
        //pbuf[i] = tmp * 22767;
        //pbuf[i+1] = tmp * 22767;
    }
    return 0;
}

void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{

    STM_EVAL_LEDOn(LED4);
    if(nextbuf == 0) { 
        Audio_MAL_Play((uint32_t)buf0, MY_BUFSIZE);
        nextbuf = 1;
    } else {
        Audio_MAL_Play((uint32_t)buf1, MY_BUFSIZE);
        nextbuf = 0;
    }
}

void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{  
}

void EVAL_AUDIO_Error_CallBack(void* pData)
{
    while (1);
}

uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
    return 0;
}
