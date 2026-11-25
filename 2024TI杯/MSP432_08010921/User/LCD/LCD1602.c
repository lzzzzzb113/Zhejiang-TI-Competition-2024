#include		"LCD1602.h"




//--------------------------------------------------------------------
/*
	LCD1602_Delay
*/
//--------------------------------------------------------------------
void LCD1602_Delay(void)
{
	uint16_t i , j ;
	
	for( i = 0 ; i < 100 ; i++ )
	{
		for( j = 0 ; j < 500 ; j++ ) ;
	}
}
//--------------------------------------------------------------------
/*
	LCD1602_Init
	RS	RW	E	D0	D1	D2	D3	D4	D5	D6	D7	BG
	PL5	PL0	PL1	PL2	PL3	PP2	PN3	PN2	PD0	PD1	PH2	PH3
*/
//--------------------------------------------------------------------
void LCD1602_Init(void)
{
	// RS 	PL5
	// Enable the GPIO port that is used for the LED.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_5);
	// RW	PL0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0);
	// E	PL1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_1);
	// D0	PL2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_2);
	// D1	PL3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_3);
	// D2 	PP2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_2);
	// D3	PN3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_3);
	// D4	PN2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_2);
	// D5	PD0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0);
	// D6 	PD1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1);
	// D7	PH2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_2);
	// BG	PH3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH); //PN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) ;
	GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_3);
}

//-----------------------------------------------------------------------
/*
	RS	RW	E	D0	D1	D2	D3	D4	D5	D6	D7	BG
	PL5	PL0	PL1	PL2	PL3	PP2	PN3	PN2	PD0	PD1	PH2	PH3
*/
//-----------------------------------------------------------------------
void LCD1602_PutData( uint8_t dat )
{	
	if( dat & 0x01 )
	{
		GPIOL->DATA |= BIT2 ;  
	}
	else
	{
		GPIOL->DATA &= ~BIT2 ;  
	}
	
	if( dat & 0x02 )
	{
		GPIOL->DATA |= BIT3 ;  
	}
	else
	{
		GPIOL->DATA &= ~BIT3 ;  
	}
	
	if( dat & 0x04 )
	{
		GPIOP->DATA |= BIT2 ;  
	}
	else
	{
		GPIOP->DATA &= ~BIT2 ;  
	}
	
	if( dat & 0x08 )
	{
		GPION->DATA |= BIT3 ;  
	}
	else
	{
		GPION->DATA &= ~BIT3 ;  
	}
	
	if( dat & 0x10 )
	{
		GPION->DATA |= BIT2 ;  
	}
	else
	{
		GPION->DATA &= ~BIT2 ;  
	}
	
	if( dat & 0x20 )
	{
		GPIOD->DATA |= BIT0 ;  
	}
	else
	{
		GPIOD->DATA &= ~BIT0 ;  
	}
	
	if( dat & 0x40 )
	{
		GPIOD->DATA |= BIT1 ;  
	}
	else
	{
		GPIOD->DATA &= ~BIT1 ;  
	}
	
	if( dat & 0x80 )
	{
		GPIOH->DATA |= BIT2 ;  
	}
	else
	{
		GPIOH->DATA &= ~BIT2 ; 
	}
}

//-----------------------------------------------------------------------
/*
	LCD_Write_Com
*/
//-----------------------------------------------------------------------
void LCD_Write_Com(uint8_t com)
{
	LCD_RS_L ;
	LCD_RW_L ;
	LCD_E_H ;
	LCD1602_PutData( com ) ;
	__NOP();
	LCD_E_L ;
	
	LCD1602_Delay() ;
}

void LCD_Write_Data(uint8_t Data)
{
	LCD_RS_H ; 
	LCD_RW_L ;
	LCD_E_H ;
	LCD1602_PutData( Data ) ;
	__NOP();
	LCD_E_L ;
	LCD1602_Delay() ;
}



//-----------------------------------------------------------------------
/*
	LCD1602_Init
*/
//-----------------------------------------------------------------------
void LCD1602_REG_Init( void )
{
	LCD1602_Init() ;
	LCD_BK_ON ;
	
	LCD_Write_Com(0x38);//显示模式设置
	LCD_Write_Com(0x01);//清屏，地址计数器清零，光标复位，显示缓冲区DDRAM的内容全部写入空格
	LCD_Write_Com(0x0c);//显示开启，不显示光标，光标不闪烁
	LCD_Write_Com(0x06);
	
}

//-----------------------------------------------------------------------
/*
	LCD_Disp
*/
//-----------------------------------------------------------------------
void LCD_Disp_Str( uint8_t Add , char *ptr_str )
{
	LCD_Write_Com(Add);
	
	while( *ptr_str != 0 )
	{
		LCD_Write_Data(*ptr_str);
		ptr_str++ ;
	}
}
struct STR_EPARA	LCD_Epara ;

//-----------------------------------------------------------------------
/*
	Epara_Update
*/
//-----------------------------------------------------------------------
void Epara_Update( struct STR_EPARA *ptr_tem_epara )
{	
	memcpy( &LCD_Epara , ptr_tem_epara , sizeof(LCD_Epara) ) ;
}

struct STR_EPARA	test_epara ;

void test_update_epara(void)
{
	test_epara.U = 22300 ;
	test_epara.I = 4310 ;
	test_epara.P = 6000 ;
	test_epara.PF = 1000 ;
	
	test_epara.THD[0] = 3000 ;
	test_epara.THD[1] = 2800 ;
	test_epara.THD[2] = 2700 ;
	test_epara.THD[3] = 2600 ;
	test_epara.THD[4] = 2500 ;
	test_epara.THD[5] = 2400 ;
	test_epara.THD[6] = 2300 ;
	test_epara.THD[7] = 2200 ;
	test_epara.THD[8] = 2100 ;
	test_epara.THD[9] = 200 ;
	test_epara.THD[10] = 1900 ;
	
	test_epara.CT_Bn = 1 ;
	
	Epara_Update( &test_epara ) ;
}

//-----------------------------------------------------------------------
/*
	Task_LCD_Run		500ms
*/
//-----------------------------------------------------------------------
uint8_t LCD_Disp_Page = 0 ;
void Task_LCD_Run( void )
{
	char str[4][16] ;	
	switch( LCD_Disp_Page )
	{
		case 0 :				// 电压		电流
			sprintf( str[0] , "U:%3.2fV    CTn"  , LCD_Epara.U / 100.0    ) ;
			sprintf( str[1] , "I:%1.3fA     %02d"  , LCD_Epara.I / 1000.0 , LCD_Epara.CT_Bn ) ;			
			
			break ;
		case 1 :
			sprintf( str[0] , "P:%1.3fW     CTn",  LCD_Epara.P / 1000.0   ) ;
			sprintf( str[1] , "PF:%1.3f     %02d"  , LCD_Epara.PF / 1000.0 ,  LCD_Epara.CT_Bn ) ;			
			break ;
		
		case 2 :
			sprintf( str[0] , "THD_A:%04.1f%%  CTn",   LCD_Epara.THD[0] / 10.0   ) ;
			sprintf( str[1] , "HAR_1:%1.3f  %02d",  LCD_Epara.THD[1] / 1000.0 ,  LCD_Epara.CT_Bn ) ;			
			break ;	
		case 3 :
			sprintf( str[0] , "HAR_2:%1.3f  CTn",  LCD_Epara.THD[2] / 1000.0   ) ;
			sprintf( str[1] , "HAR_3:%1.3f  %02d",  LCD_Epara.THD[3] / 1000.0 ,  LCD_Epara.CT_Bn  ) ;			
			break ;
		case 4 :
			sprintf( str[0] , "HAR_4:%1.3f  CTn",  LCD_Epara.THD[4] / 1000.0   ) ;
			sprintf( str[1] , "HAR_5:%1.3f  %02d",  LCD_Epara.THD[5] / 1000.0 ,  LCD_Epara.CT_Bn  ) ;			
			break ;
		case 5 :
			sprintf( str[0] , "HAR_6:%1.3f  CTn",  LCD_Epara.THD[6] / 1000.0   ) ;
			sprintf( str[1] , "HAR_7:%1.3f  %02d",  LCD_Epara.THD[7] / 1000.0 ,  LCD_Epara.CT_Bn  ) ;			
			break ;
		case 6 :
			sprintf( str[0] , "HAR_8:%1.3f  CTn",  LCD_Epara.THD[8] / 1000.0  ) ;
			sprintf( str[1] , "HAR_9:%1.3f  %02d",  LCD_Epara.THD[9] / 1000.0 ,  LCD_Epara.CT_Bn  ) ;			
			break ;
		case 7 :
			sprintf( str[0] , "HAR_10:%1.3f CTn",  LCD_Epara.THD[10] / 1000.0   ) ;
			sprintf( str[1] , "             %02d " ,   LCD_Epara.CT_Bn ) ;			
			break ;
		default :
			LCD_Disp_Page = 0 ;
		
	}
	
	LCD_Disp_Str( 0x80 , str[0] ) ;		
	LCD_Disp_Str( 0xC0 , str[1] ) ;
}





