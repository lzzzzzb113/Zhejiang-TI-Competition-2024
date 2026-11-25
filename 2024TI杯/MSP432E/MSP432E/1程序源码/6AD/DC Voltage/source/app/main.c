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
uint16_t dataBufferA[1];
uint16_t dataBufferB[1];
volatile bool bgetConvStatus = false;;
uint16_t adc_buff[FFT_LENGTH];//存放ADC采集的数据

/* Private function prototypes -----------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	uart0_init();
	adc_init();
	
	printf("Begin\r\n");
	
	while(1)
	{
		MAP_ADCProcessorTrigger(ADC1_BASE, (ADC_TRIGGER_WAIT | 3));
		MAP_ADCProcessorTrigger(ADC0_BASE, (ADC_TRIGGER_SIGNAL | 3));

		/* Wait for the conversion to complete */
		while(!bgetConvStatus);
		bgetConvStatus = false;

		/* Display the digital value on the console. */
		printf("V = %0.4fv\r\n", dataBufferA[0]* 3.3 / 4095);
		delay_ms(500);		
	}
}
