#ifndef __ADC_H_
#define	__ADC_H_

/* DriverLib Includes */
#include <driverlib.h>

//电压放大倍数
#define VOLTAGE_GAIN 1.0f
//电流放大倍数
#define CURRENT_GAIN 1.0f


#define START_czk_Log 1

#define Vref 3.3f
#define ADC_RESOLUTION 12

/* Define for Samples to be captured */
#define NUM_SAMPLES 1024
#define NUM_SAMPLES_BIT 10
#define SampleRate 5120 // 1024*5 = 5120

/* DMA Buffer declaration and buffer complet flag */
extern uint32_t dstBufferA[NUM_SAMPLES];
extern uint32_t dstBufferB[NUM_SAMPLES];

void ConfigureADC0(uint32_t systemClock);
void ConfigureADC1(uint32_t systemClock);
#endif /* __ADC_H_ */
