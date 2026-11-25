#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "driverlib.h"

#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#define X_MAX_PIXEL	        128
#define Y_MAX_PIXEL	        128

//-----------------LCD端口定义---------------- 

//液晶控制口置1操作语句宏定义
#define	LCD_SCL_SET  	GPIOB->DATA |= BIT4   
#define	LCD_SDA_SET  	GPIOB->DATA |= BIT5 
#define	LCD_CS_SET  	GPIOK->DATA |= BIT2
#define	LCD_LED_SET  	GPIOK->DATA |= BIT3  
#define	LCD_RS_SET  	GPIOK->DATA |= BIT1
#define	LCD_RST_SET  	GPIOK->DATA |= BIT0

//液晶控制口置0操作语句宏定义
#define	LCD_SCL_CLR  	GPIOB->DATA &= ~BIT4   
#define	LCD_SDA_CLR  	GPIOB->DATA &= ~BIT5 
#define	LCD_CS_CLR  	GPIOK->DATA &= ~BIT2
#define	LCD_LED_CLR  	GPIOK->DATA &= ~BIT3  
#define	LCD_RST_CLR  	GPIOK->DATA &= ~BIT0
#define	LCD_RS_CLR  	GPIOK->DATA &= ~BIT1

#define LCD_WR_DATA8 		Lcd_WriteIndex
#define LCD_Address_Set Lcd_SetRegion
#define LCD_WR_DATA 		LCD_WriteData_16Bit

void delay_ms(int numtodelay);

void LCD_Init(void);
void Lcd_WriteIndex(u8 Index);//写入一个指令
void Lcd_WriteData(u8 Data);//写入一个字节
void LCD_WriteData_16Bit(u16 Data);//写入两个字节
void Lcd_WriteReg(u8 Index,u8 Data);//写一个寄存器
void Lcd_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end);
#endif

