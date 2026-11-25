/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "uart.h"
#include "uartstdio.h"

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;
uint8_t ReceiveData[1];

/* Private function prototypes -----------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	uart0_init();
	uart2_init();
	
	UARTSend(UART0_BASE, "Begin\r\n", 7);
	
	while(1)
	{	
		
	}
}

//串口屏接受中断
void UART2_IRQHandler(void)
{
	uint32_t ui32Status;
	
	ui32Status = MAP_UARTIntStatus(UART2_BASE, true);
	MAP_UARTIntClear(UART2_BASE, ui32Status);
	while(MAP_UARTCharsAvail(UART2_BASE))
	{
		ReceiveData[0] = MAP_UARTCharGetNonBlocking(UART2_BASE);//HWREG(UART2_BASE + 0x00000000);
		MAP_UARTCharPutNonBlocking(UART2_BASE, MAP_UARTCharGetNonBlocking(UART2_BASE));
//代码区
//		printf("ReceiveData = %d\r\n", ReceiveData[0]);
		switch(ReceiveData[0]-48)
		{
			case  0: printf("0\r\n");break;
			case  1: printf("1\r\n");break;
			default: printf("Receive Error\r\n");break;
		}
	}
}
