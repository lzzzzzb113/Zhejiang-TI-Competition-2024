#define __LCD1602_H

#include"msp432e401y.h"
#include"gpio.h"

#define  LCD1602_Timer_GPIOA TIMER0_BASE
#define  LCD1602_Timer_GPIOB TIMER1_BASE
#define  LCD1602_GPIOA GPIO_PORTA_BASE
#define  LCD1602_GPIOB GPIO_PORTB_BASE
#define  LCD1602_RW   GPIO_PIN_0
#define  LCD1602_RS   GPIO_Pin_1
#define  LCD1602_E    GPIO_Pin_2
#define  LCD1602_IO   GPIO_Pin_3 |  GPIO_Pin_4 |  GPIO_Pin_5 |  GPIO_Pin_6 |  GPIO_Pin7 |  GPIO_Pin_13 |  GPIO_Pin_14 |  GPIO_Pin_15

#define  LCD1602_I0_SET() GPIO_SetBits(GPIOB, LCD1602_IO)

#define  LCD1602_RS_SET()   GPIO_SetBits(GPIOA,LCD1602_RS)
#define  LCD1602_RS_RESET() GPIO_ResetBits(GPIOA_,LCD1602_RS)

#define  LCD1602_RW_SET()   GPIO_SetBits(GPIOB,LCD1602_RW)
#define  LCD1602_RW_RESET() GPIO_ResetBits(GPIOB,LCD1602_RW)

#define  LCD1602_E_SET()   GPIO_SetBits(GPIOB,LCD1602_E)
#define  LCD1602_E_RESET() GPIO_ResetBits(GPIOB,LCD1602_E)

   

void LCD1602_Init(void);//液晶初始化函数
void LCD1602_ShowNum(unsigned char x,unsigned char y,unsigned char *str,unsigned char i);//LCD1602显示数字
void LCD1602_ShowStr(unsigned char x,unsigned char y,unsigned char *str);//LCD1602显示字符串
