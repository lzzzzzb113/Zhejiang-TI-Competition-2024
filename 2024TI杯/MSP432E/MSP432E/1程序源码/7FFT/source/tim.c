#include "tim.h"

extern uint32_t ui32SysClock;

void capture_init()         // GPIO PA7-----  Timer-3 CCP1 pin           
{
   MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)))
    {
    }

    /* Configure the GPIO PA7 as Timer-3 CCP1 pin */
    MAP_GPIOPinConfigure(GPIO_PA7_T3CCP1);
    MAP_GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7);

    /* Enable the Timer-3 in 16-bit Edge Time mode */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3)))
    {
    }

    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME_UP);
    MAP_TimerLoadSet(TIMER3_BASE, TIMER_B, ui32SysClock); 
    
		MAP_TimerIntEnable(TIMER3_BASE, TIMER_CAPB_EVENT);
    MAP_IntEnable(INT_TIMER3B);

    MAP_TimerControlEvent(TIMER3_BASE, TIMER_B, TIMER_EVENT_NEG_EDGE);
    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME_UP);

    MAP_TimerEnable(TIMER3_BASE, TIMER_B);
    /* Enable the timer interrupt */
}

void tim0_init()
{
    /* 使能定时器0的时钟并且设置频率为1KHz，使能ADC触发方式为定时器0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)))
    {
    }
 
    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, (ui32SysClock/102400));
		
    MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
		
//    MAP_TimerEnable(TIMER0_BASE, TIMER_A);
}

void tim1_init()
{
   // Enable the GPIO port that is used for the on-board LEDs.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    // Enable the peripherals used by this example.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);	
    // Enable processor interrupts.
    //
    MAP_IntMasterEnable();

    //
    // Configure the two 32-bit periodic timers.
    //
    MAP_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, ui32SysClock/2);//此处分频
    //
    // Setup the interrupts for the timer timeouts.
    //
    MAP_IntEnable(INT_TIMER1A);
    MAP_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the timers.
    //
    MAP_TimerEnable(TIMER1_BASE, TIMER_A);
}


