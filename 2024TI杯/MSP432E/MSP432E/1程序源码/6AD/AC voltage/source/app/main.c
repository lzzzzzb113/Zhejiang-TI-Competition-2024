/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "uart.h"
#include "uartstdio.h"
#include "tim.h"
#include "adc.h"

/* Private define ------------------------------------------------------------*/
#define FFT_LENGTH 1024 

/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;
uint16_t i;
extern uint16_t dataBufferA[1];
extern uint16_t dataBufferB[1];
extern volatile bool bgetConvStatus;;
uint16_t adc_buff[FFT_LENGTH];//存放ADC采集的数据
volatile bool AdcConvEnd;//用来检测ADC是否采集完毕 0：没有采集完毕 1：采集完毕

/* Private function prototypes -----------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	uart0_init();
	tim0_init();
	adc0_init();
	
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
//显示波形	
		for(i=0; i<FFT_LENGTH; i++)
		{
			printf("AD:%0.3f\r\n", adc_buff[i]*3.3/4095 );
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
