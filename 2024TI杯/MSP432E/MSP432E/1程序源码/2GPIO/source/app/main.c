/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "led.h"
#include "button.h"

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;
uint8_t button_status;

/* Private function prototypes -----------------------------------------------*/
void delay(void);

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	led_init();
	buttons_init();
	
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00);
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);

	while(1)
	{
		button_status = !(GPIOPinRead(GPIO_PORTJ_BASE, BUTTON1_PIN) & BUTTON1_PIN);
		if(button_status)
    {
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
		}
		else
		{
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00);
		}
		delay();
	}
}

void delay()
{
	int i=0xffffff;
	while(i--);
}
