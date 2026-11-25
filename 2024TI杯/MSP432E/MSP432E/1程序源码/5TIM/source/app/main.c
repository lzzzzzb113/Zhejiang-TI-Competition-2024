/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "uart.h"
#include "uartstdio.h"
#include "tim.h"

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;
uint32_t Count;
/* Private function prototypes -----------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	uart0_init();
	tim1_init();
	capture_init();
	
	printf("Begin\r\n");
	
	while(1)
	{
		
	}
}

void TIMER1A_IRQHandler(void)
{
	MAP_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
//	printf("1\r\n");
	printf("Count:%d\r\n",Count);
	Count = 0;
}

void TIMER3B_IRQHandler(void)
{
	uint32_t getTimerIntStatus;
	getTimerIntStatus = MAP_TimerIntStatus(TIMER3_BASE, true);
	MAP_TimerIntClear(TIMER3_BASE, getTimerIntStatus);
//´úÂëÇø
	Count++;
}
