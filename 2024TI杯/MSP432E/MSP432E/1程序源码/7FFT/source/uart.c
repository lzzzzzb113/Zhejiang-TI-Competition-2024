#include "uart.h"

uint8_t u_buf[256];//printf需要的数组
extern uint32_t ui32SysClock;


void UARTSend(uint32_t ui32Base, const uint8_t *pui8Buffer, uint32_t ui32Count)
{
		while(ui32Count--)
		{
				MAP_UARTCharPutNonBlocking(ui32Base, *pui8Buffer++);//发送字符串的函数
		}
}


void uart0_init(void)
{
    /* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART for 115200 bps 8-N-1 format */
    MAP_UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
	
//    /* Enable the clock to GPIO port A and UART 0 */
//    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
//    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

//    /* Configure the GPIO Port A for UART 0 */
//    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
//    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
//    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

//    /* Configure the UART for 115200 bps 8-N-1 format */
//    UARTStdioConfig(0, 115200, ui32SysClock);
}

void uart2_init(void)
{
    /* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    //
    // Enable processor interrupts.
    //
    MAP_IntMasterEnable();

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PD4_U2RX);
    MAP_GPIOPinConfigure(GPIO_PD5_U2TX);
    MAP_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    /* Configure the UART for 115200 bps 8-N-1 format */
    UARTStdioConfig(2, 115200, ui32SysClock);

    //
    // Enable the UART interrupt.
    //
    MAP_IntEnable(INT_UART2);
    MAP_UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT);
		/*UART 可用的中断类型：
													UART_INT_9BIT - 9位地址匹配中断
													UART_INT_OE - 溢出错误中断
													UART_INT_BE - 中断错误中断
													UART_INT_PE - 奇偶校验错误中断
													UART_INT_FE - 帧错误中断
													UART_INT_RT - 接收超时中断
													UART_INT_TX - 发送中断
													UART_INT_RX - 接收中断
													UART_INT_DSR - DSR中断
													UART_INT_DCD - DCD中断
													UART_INT_CTS - CTS中断
													UART_INT_RI - RI中断
		****************************************************************/
}










