#include "main.h"
#include "soundpipe.h"

#define MY_BUFSIZE 64

static volatile uint8_t nextbuf;
static int16_t buf0[MY_BUFSIZE];
static int16_t buf1[MY_BUFSIZE];

/* SOUNDPIPE */

#define NOSCS 8
static const uint8_t scale[] = {48, 60, 65, 67, 72, 79, 64, 52};

//static const uint8_t scale[] = {48, 62, 65, 67, 72, 79, 64, 50};

static uint8_t please_play = 0;
static sp_data *sp;
static sp_ftbl *ft;
static sp_osc *osc[NOSCS];
static sp_revsc *revsc;

volatile uint8_t state = 0;
volatile uint32_t base_frequency = 500;

uint32_t compute_buffer(int16_t *pbuf, int  bufsize);

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
//    sp->len = AudioFreq * 5;

    sp_ftbl_create(sp, &ft, 8192);
    sp_gen_sine(sp, ft);

    uint32_t i;
    float base_amplitude = 0.9f;

    for(i = 0; i < NOSCS; i++) {
        sp_osc_create(&osc[i]);
        sp_osc_init(sp, osc[i], ft, 0);
        osc[i]->freq = (uint32_t)(base_frequency * (i+1));
        osc[i]->amp = (float)(base_amplitude / (i+1));
    }

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
    base_frequency = (uint32_t)adc_convert();
    for(i = 0; i < bufsize / 2; i+=2) {
        fm = 0;
        tmp = 0;
        for(o = 0; o < NOSCS; o++) {
            osc[o]->freq = (uint32_t)(base_frequency * (o+1));
            sp_osc_compute(sp, osc[o], NULL, &tmp);
            fm += tmp;
        }
        pbuf[i] = fm * 32767 / 1.7f;
        pbuf[i+1] = fm * 32767 / 1.7f;
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
    STM_EVAL_LEDOn(LED5);
}

void EVAL_AUDIO_Error_CallBack(void* pData)
{
    while (1);
}

uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
    return 0;
}
