/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "uart.h"
#include "uartstdio.h"
#include "ADS8688.h"

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern uint32_t ui32SysClock;
uint32_t i;
uint16_t ADC_Vol[2];

/* Private function prototypes -----------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	uart0_init();
	ADS8688_Init(CH0_EN | CH1_EN);//ADS8688初始化
	
	printf("Begin\r\n");

  Set_CH_Range(CHIR_0, ADS8688_IR_N2_5V);//设置通道输入范围：±10.24V ±2.5*ref  ref=4.096V.通道每个量程配置可相同也可不同。
	Set_CH_Range(CHIR_1, ADS8688_IR_N2_5V);
	
	while(1)
	{
		MAN_CH_Mode(MAN_CH_0);					//选择通道
		ADC_Vol[0] = Get_MAN_CH_Mode_Data();
//		MAN_CH_Mode(MAN_CH_1);
//		ADC_Vol[1] =Get_MAN_CH_Mode_Data();//读取通道数据
		
		printf("AD0:%.3f\r\n", ADC_Vol[0]* 20.48 / 65536 - 10.24);
//		printf("AD1:%.3f\r\n", ADC_Vol[1]* 20.48 / 65536 - 10.24);
		delay_ms(500);	
	}
}

