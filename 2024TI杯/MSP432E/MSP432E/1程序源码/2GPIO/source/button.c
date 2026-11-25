#include "button.h"

void buttons_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ))
	{
	}

	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, BUTTON1_PIN | BUTTON2_PIN);
	GPIOPadConfigSet(GPIO_PORTJ_BASE, BUTTON1_PIN | BUTTON2_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}


uint8_t buttons_read(void)
{
	uint8_t button1_status = !(GPIOPinRead(GPIO_PORTJ_BASE, BUTTON1_PIN) & BUTTON1_PIN);
	uint8_t button2_status = !(GPIOPinRead(GPIO_PORTJ_BASE, BUTTON2_PIN) & BUTTON2_PIN);

	return (button2_status << 1) | button1_status;
}
