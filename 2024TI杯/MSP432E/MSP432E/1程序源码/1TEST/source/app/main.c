/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;

/* Private function prototypes -----------------------------------------------*/
void delay(void);

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
	while(1)
	{
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
		delay();
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
		delay();		
	}
}

void delay()
{
	int i=0xffffff;
	while(i--);
}
