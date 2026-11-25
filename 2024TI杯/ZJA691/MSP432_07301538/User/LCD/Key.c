#include 	"Key.h"

uint8_t flag_Tmr_1ms = 0 ;

void Timer2_Init(void)
{
	uint32_t systemClock;
	
	/* Configure the system clock for 120 MHz */
	systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
	
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	while (!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2))) {
	}
	MAP_TimerConfigure(TIMER2_BASE, TIMER_CFG_A_PERIODIC);
	MAP_TimerLoadSet(TIMER2_BASE, TIMER_A, (systemClock / 1000)); //czk
	MAP_TimerEnable(TIMER2_BASE, TIMER_A);
	

	
	TimerIntEnable( TIMER2_BASE , TIMER_TIMA_TIMEOUT );
	
	IntEnable(INT_TIMER2A);
}


void TIMER2A_IRQHandler(void)
{
	uint32_t getIntStatus;
	
	getIntStatus = TimerIntStatus( TIMER2_BASE , TIMER_TIMA_TIMEOUT ) ;
	TimerIntClear(TIMER2_BASE , TIMER_TIMA_TIMEOUT) ;
	
	if(getIntStatus == 1)
  {}
  
	flag_Tmr_1ms = 1 ;
	
}


//--------------------------------------------------------------------
/*
	Key_Init
	Key1 	Key2	Key3	Key4	key5	Key6	
	PP3		PA7		PQ1		PQ2		PM6		PQ3
*/
//--------------------------------------------------------------------
void Key_Init(void)
{
	// Key1 PP3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP)) ;	
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_3);
	HWREG(GPIO_PORTP_BASE + GPIO_O_PUR) |= ( 0x01 << 3 ) ;
	
	// Key2	PA7
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) ;	
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);
	HWREG(GPIO_PORTA_BASE + GPIO_O_PUR) |= ( 0x01 << 7 ) ;
	// Key3 PQ1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ)) ;	
	GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_1);
	HWREG(GPIO_PORTQ_BASE + GPIO_O_PUR) |= ( 0x01 << 1 ) ;
	// Key4 PQ2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ)) ;	
	GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_2);
	HWREG(GPIO_PORTQ_BASE + GPIO_O_PUR) |= ( 0x01 << 2 ) ;
	// Key5 PM6
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)) ;	
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_6);
	HWREG(GPIO_PORTM_BASE + GPIO_O_PUR) |= ( 0x01 << 6 ) ;
	// Key6	PQ3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ)) ;	
	GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_3);
	HWREG(GPIO_PORTQ_BASE + GPIO_O_PUR) |= ( 0x01 << 3 ) ;
	
	
	
	
	
}


//--------------------------------------------------------------------
/*
		Task_Key_Run				10ms
	Key1 	Key2	Key3	Key4	key5	Key6	
	PP3		PA7		PQ1		PQ2		PM6		PQ3
*/
//--------------------------------------------------------------------
void Task_Key_Run(void)
{
	static uint8_t cnt_key1_press = 0 ;
	static uint8_t cnt_key2_press = 0 ;
	static uint8_t cnt_key3_press = 0 ;
	static uint8_t cnt_key4_press = 0 ;
	static uint8_t cnt_key5_press = 0 ;
	static uint8_t cnt_key6_press = 0 ;
	
	//  Key1 PP3
	if( 0 == GPIOPinRead( GPIO_PORTP_BASE , GPIO_PIN_3 ) )
	{
		if( cnt_key1_press < 5 )
		{
			cnt_key1_press++ ;
			if( cnt_key1_press == 5 )
			{
				// Key1 pressed
				if( LCD_Disp_Page > 0 )
				{
					LCD_Disp_Page-- ;
				}
			}
		}
	}
	else
	{
		cnt_key1_press = 0 ;
	}
	
	// Key2	PA7
	if( 0 == GPIOPinRead( GPIO_PORTA_BASE , GPIO_PIN_7 ) )
	{
		if( cnt_key2_press < 5 )
		{
			cnt_key2_press++ ;
			if( cnt_key2_press == 5 )
			{
				// Key2 pressed
				if( LCD_Disp_Page < 8 )
				{
					LCD_Disp_Page++ ;
				}
				
			}
		}
	}
	else
	{
		cnt_key2_press = 0 ;
	}
	// Key3 PQ1
	if( 0 == GPIOPinRead( GPIO_PORTQ_BASE , GPIO_PIN_1 ) )
	{
		if( cnt_key3_press < 5 )
		{
			cnt_key3_press++ ;
			if( cnt_key3_press == 5 )
			{
				// Key3 pressed
				if( LCD_Epara.CT_Bn > 1 )
				{
					LCD_Epara.CT_Bn-- ;
				}
			}
		}
	}
	else
	{
		cnt_key3_press = 0 ;
	}
	// Key4 PQ2
	if( 0 == GPIOPinRead( GPIO_PORTQ_BASE , GPIO_PIN_2 ) )
	{
		if( cnt_key4_press < 5 )
		{
			cnt_key4_press++ ;
			if( cnt_key4_press == 5 )
			{
				// Key4 pressed
				if( LCD_Epara.CT_Bn < 10 )
				{
					LCD_Epara.CT_Bn++ ;
				}
			}
		}
	}
	else
	{
		cnt_key4_press = 0 ;
	}
	// Key5 PM6
	if( 0 == GPIOPinRead( GPIO_PORTM_BASE , GPIO_PIN_6 ) )
	{
		if( cnt_key5_press < 5 )
		{
			cnt_key5_press++ ;
			if( cnt_key5_press == 5 )
			{
				// Key5 pressed
				LCD_BK_TOG ;
			}
		}
	}
	else
	{
		cnt_key5_press = 0 ;
	}
	// Key6	PQ3	
	if( 0 == GPIOPinRead( GPIO_PORTQ_BASE , GPIO_PIN_3 ) )
	{
		if( cnt_key6_press < 5 )
		{
			cnt_key6_press++ ;
			if( cnt_key6_press == 5 )
			{
				// Key1 pressed
			}
		}
	}
	else
	{
		cnt_key6_press = 0 ;
	}
}


