#ifndef __SIGNALPROCESS_H_
#define	__SIGNALPROCESS_H_

#include <driverlib.h>
#include "bsp_ADC.h"
// sA[i].real存储实部，sA[i].imag存储虚部
// 开始存储时域，经过FFT后存储频域
extern struct compx sA[NUM_SAMPLES];
extern struct compx sB[NUM_SAMPLES];

#define Harmonic_Num_czk 15
extern float Tab_Harmonic[Harmonic_Num_czk][3];

void Calc_THD(float *ptr_buf_in, uint16_t len, uint16_t *ptr_THD, uint32_t *ptr_S_Frq);
void Calc_Channl_FFTValue(void);

#endif /* __SIGNALPROCESS_H_ */
