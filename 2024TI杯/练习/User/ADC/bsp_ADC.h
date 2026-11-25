#ifndef __ADC_H_
#define	__ADC_H_

/* DriverLib Includes */
#include <driverlib.h>


/* Define for Samples to be captured */
#define NUM_SAMPLES 1024


/* DMA Buffer declaration and buffer complet flag */
extern uint32_t dstBufferA[NUM_SAMPLES];

void ConfigureADC0(uint32_t systemClock);
void ConfigureADC1(uint32_t systemClock);
#endif /* __ADC_H_ */
