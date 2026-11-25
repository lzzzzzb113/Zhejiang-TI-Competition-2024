/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "uart.h"
#include "uartstdio.h"
#include "tim.h"
#include "adc.h"
#include "arm_math.h"//,ARM_MATH_CM4   --diag_suppress=2803,1,1035
#include "arm_const_structs.h"

/* Private define ------------------------------------------------------------*/
#define FFT_LENGTH 1024 

/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;
uint16_t i;

uint16_t adc_buff[FFT_LENGTH];//存放ADC采集的数据
volatile bool AdcConvEnd;//用来检测ADC是否采集完毕 0：没有采集完毕 1：采集完毕

arm_cfft_radix4_instance_f32 scfft;
float fft_inputbuf[FFT_LENGTH * 2];
float fft_outputbuf[FFT_LENGTH];
float Mag[FFT_LENGTH];

/* Private function prototypes -----------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	uart0_init();
	tim0_init();
	adc0_init();
	arm_cfft_radix4_init_f32(&scfft,FFT_LENGTH,0,1);//初始化scfft结构体，设定FFT相关参数
	
	printf("Begin\r\n");
	
	while(1)
	{
//采集电压	
		MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);
		MAP_TimerEnable(TIMER0_BASE, TIMER_A);
		while(!AdcConvEnd);
		AdcConvEnd = false;
		MAP_uDMAChannelDisable(UDMA_CH16_ADC0_2);
		MAP_TimerDisable(TIMER0_BASE, TIMER_A);
//直接输出			
		for(i = 0; i < FFT_LENGTH; i++)
		{
			fft_inputbuf[i*2] = adc_buff[i] * 3.3 / 4095;
			fft_inputbuf[i*2 + 1] = 0;
		}		
//FFT计算
		arm_cfft_radix4_f32(&scfft,fft_inputbuf);	//FFT计算（基4）				
		arm_cmplx_mag_f32(fft_inputbuf,fft_outputbuf,FFT_LENGTH);	//把运算结果复数求模得幅值 				
////显示波形	
//		for(i=0; i<FFT_LENGTH; i++)
//		{
//			printf("AD:%0.3f\r\n", adc_buff[i]*3.3/4095 );
//			delay_ms(1);
//		}
//显示频谱		
		for(i=0; i<FFT_LENGTH; i++)
		{
//				printf("%d:\t%.2f\r\n", i, fft_outputbuf[i]);
			printf("%.2f\r\n", fft_outputbuf[i]);
			delay_ms(1);
		}
		delay_ms(1000);
	}
}

void ADC0SS2_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

    /* If the interrupt status for Sequencer-2 is set the
     * clear the status and read the data */
    if((getIntStatus & ADC_INT_DMA_SS2) == ADC_INT_DMA_SS2)
    {
        /* Clear the ADC interrupt flag. */
        MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);

        /* Reconfigure the channel control structure and enable the channel */
        MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                                   UDMA_MODE_BASIC,
                                   (void *)&ADC0->SSFIFO2, (void *)&adc_buff,
                                   sizeof(adc_buff)/2);

        MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);

        /* Set conversion flag to true */
        AdcConvEnd = true;
    }
}
