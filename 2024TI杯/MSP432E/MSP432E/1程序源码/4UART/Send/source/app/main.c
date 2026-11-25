/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "uart.h"
#include "uartstdio.h"

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;
uint32_t i;

/* Private function prototypes -----------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	uart0_init();
	
	UARTSend(UART0_BASE, "Begin\r\n", 7);
	
	while(1)
	{
		for(i=0;i<1000;i++)
		{
			printf("i:%d\r\n",i);
			delay_ms(1);
		}		
	}
}

