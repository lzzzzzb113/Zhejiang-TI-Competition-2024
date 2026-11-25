#ifndef		_LCD1602_H_
#define 	_LCD1602_H_

#include		 <driverlib.h>
#include		"string.h"
#include		"stdio.h"
#include "inc/hw_gpio.h"

#define 	LCD_RS_H		GPIOL->DATA |= BIT5  
#define 	LCD_RS_L		GPIOL->DATA &= ~BIT5  

#define 	LCD_RW_H		GPIOL->DATA |= BIT0  
#define 	LCD_RW_L		GPIOL->DATA &= ~BIT0

#define 	LCD_E_H		GPIOL->DATA |= BIT1
#define 	LCD_E_L		GPIOL->DATA &= ~BIT1

#define 	LCD_BK_ON		GPIOH->DATA |= BIT3
#define 	LCD_BK_OFF		GPIOH->DATA &= ~BIT3
#define 	LCD_BK_TOG		GPIOH->DATA ^= BIT3


struct STR_EPARA
{ 
	uint16_t U ;			// 放大100倍    电压
	uint16_t I ;			// 放大1000 倍  电流
	uint16_t P ;			// 放大1倍      功率
	uint16_t PF	;			// 放大1000倍   功率因数
	uint16_t THD[11] ;		// 放大1000倍
	
	uint16_t CT_Bn	;		// 电流互感器原方匝数
} ;

extern uint8_t LCD_Disp_Page ;
extern struct STR_EPARA	LCD_Epara ; 

void LCD1602_REG_Init( void ) ;
void LCD_Disp_Str( uint8_t Add , char *ptr_str ) ;
void Task_LCD_Run( void ) ;
void test_update_epara(void) ;


#endif
