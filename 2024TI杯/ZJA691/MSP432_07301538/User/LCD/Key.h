#ifndef		_KEY_H_
#define 	_KEY_H_

#include		 <driverlib.h>
#include "inc/hw_gpio.h"
#include 	"LCD1602.h"

extern uint8_t flag_Tmr_1ms ;

void Key_Init(void) ;
void Task_Key_Run(void) ;
void Timer2_Init(void) ;


#endif 

