/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "uart.h"
#include "uartstdio.h"
#include "ADS112C04_i2c.h"
#include "ADS112C04.h"

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;
uint32_t i;
float ADC_Vol;

/* Private function prototypes -----------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	uart0_init();
	ADS112_Init();
	
	printf("Begin\r\n");
	
	while(1)
	{
		ADS112_Send_Command(ADS122C04_START_CMD);
		ADC_Vol=ADS112_GetADC_Vol();
		printf("ADC_Vol:%0.3fV\r\n",ADC_Vol);
		delay_ms(500);	
	}
}

