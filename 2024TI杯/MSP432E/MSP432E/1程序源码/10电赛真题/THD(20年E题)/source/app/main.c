/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include <stdint.h>
#include <stdbool.h>
#include "stdio.h"
#include "delay.h"
#include "system_init.h"
#include "led.h"
#include "button.h"
#include "uart.h"
#include "tim.h"
#include "adc.h"
#include "ADS112C04_i2c.h"
#include "ADS112C04.h"
#include "uartstdio.h"
#include "arm_math.h"//,ARM_MATH_CM4   --diag_suppress=2803,1,1035
#include "arm_const_structs.h"
#include "Kalman.h"

/* Private define ------------------------------------------------------------*/
#define FFT_LENGTH 1024  
#define Num_Sample 20

#define Switch1_GPIO	GPIO_PORTC_BASE
#define Switch2_GPIO	GPIO_PORTC_BASE
#define Switch3_GPIO	GPIO_PORTE_BASE
#define Switch4_GPIO	GPIO_PORTD_BASE
#define Switch5_GPIO	GPIO_PORTC_BASE
#define Switch6_GPIO	GPIO_PORTB_BASE
#define Switch7_GPIO	GPIO_PORTB_BASE

#define Switch1_PIN		GPIO_PIN_5
#define Switch2_PIN		GPIO_PIN_6
#define Switch3_PIN		GPIO_PIN_5
#define Switch4_PIN		GPIO_PIN_3
#define Switch5_PIN		GPIO_PIN_7
#define Switch6_PIN		GPIO_PIN_2
#define Switch7_PIN		GPIO_PIN_3

/* Private variables ---------------------------------------------------------*/
uint32_t ui32SysClock;
uint8_t ReceiveData[1];
uint8_t j;
uint16_t i;

uint16_t adc_buff[FFT_LENGTH];//存放ADC采集的数据
volatile bool AdcConvEnd = false;//用来检测ADC是否采集完毕 0：没有采集完毕 1：采集完毕
arm_cfft_radix4_instance_f32 scfft;
float fft_inputbuf[FFT_LENGTH * 2];
float fft_outputbuf[FFT_LENGTH];
float Mag[FFT_LENGTH];
float THD;

uint8_t Page = 0,K1 = 1,K2 = 1,Type = 1;
uint8_t ReceiveData[1];
uint8_t Type_Auto = 0;

uint32_t fadc = 102400;
uint32_t fin;

volatile bool f_get_flag = false;//用来检测ADC是否采集完毕 0：没有采集完毕 1：采集完毕

uint8_t Page_Before = 0;

Kalman kfp;

/* Private function prototypes -----------------------------------------------*/
void THD_and_level(uint8_t NUM);
void K1_ON(void);
void K1_OFF(void);
void K2_ON(void);
void K2_OFF(void);
void Type_Wu(void);
void Type_Ding(void);
void Type_Di(void);
void Type_Shuang(void);
void Type_Jiao(void);
void Switch_GPIO_Config(void);
void Dynamically_Adjus(uint32_t f);
void THD_and_level_withCompensate(uint8_t NUM);

/* Main ----------------------------------------------------------------------*/
int main(void)
{
//初始化
	sys_init();
	uart0_init();
//	led_init();
//	buttons_init();
	uart2_init();
//	tim1_init();
//	adc_init();
	adc0_init();
	tim0_init();
	tim1_init();
	capture_init(); 
//	ADS112_Init();
	Switch_GPIO_Config();
	Kalman_Init();
	
	printf("Begin\r\n");
	arm_cfft_radix4_init_f32(&scfft,FFT_LENGTH,0,1);//初始化scfft结构体，设定FFT相关参数
	K1_ON();
	K2_ON();	
	printf2f("t2.txt=\"%0.3f\"\xff\xff\xff",1.0);
	delay_ms(5);
	printf2f("t6.txt=\"%0.3f\"\xff\xff\xff",2.5);
	while(1)
	{
//改变页数
		if(Page_Before != Page)	
		{
			Type_Auto = 0;
			K1_ON();
			K2_ON();
			THD = 0;
			j = 0;
		}
		
		
//基础要求
		if(Page == 1)
		{
//初始化			
				Page_Before	= Page;		
//按键操作
				switch(K1)
				{
					case 1: K1_ON();break;
					case 2: K1_OFF();break;
					default:printf("K1 error\r\n");
				}
				switch(K2)
				{
					case 1: K2_ON();break;
					case 2: K2_OFF();break;
					default:printf("K2 error\r\n");break;
				}
				delay_ms(1);
				switch(Type)
				{
					case 1: Type_Wu();break;
					case 2: Type_Ding();break;
					case 3: Type_Di();break;
					case 4: Type_Shuang();break;
					case 5: Type_Jiao();break;
					default:printf("Type error\r\n");break;
				}
//采集电压	
				MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);
				MAP_TimerEnable(TIMER0_BASE, TIMER_A);
				while(!AdcConvEnd);
				AdcConvEnd = false;
				MAP_uDMAChannelDisable(UDMA_CH16_ADC0_2);
				MAP_TimerDisable(TIMER0_BASE, TIMER_A);
//卡尔曼滤波
				for(i = 0; i < FFT_LENGTH; i++)
				{
					adc_buff[i] = KalmanFilter(&kfp,adc_buff[i]);
				}					
//直接输出			
				for(i = 0; i < FFT_LENGTH; i++)
				{
					fft_inputbuf[i*2] = adc_buff[i] * 3.3 / 4095;
					fft_inputbuf[i*2 + 1] = 0;
				}		
//FFT计算
				arm_cfft_radix4_f32(&scfft,fft_inputbuf);	//FFT计算（基4）				
				arm_cmplx_mag_f32(fft_inputbuf,fft_outputbuf,FFT_LENGTH);	//把运算结果复数求模得幅值 				
////显示波形	
//				for(i=0; i<200; i++)
//				{
//					printf("%0.3f\r\n", adc_buff[i]*3.3/4095 );
//					delay_ms(1);
//				}
//				delay_ms(1000);
////显示频谱		
//				for(i=0; i<512; i++)
//				{
//		//				printf("%d:\t%.2f\r\n", i, fft_outputbuf[i]);
//					printf("%.2f\r\n", fft_outputbuf[i]);
//					delay_ms(1);
//				}	
//THD		
//				THD_and_level(1);
//				printf("THD = %0.2f\r\n",THD);
//				HAL_Delay(500);
						
				THD_and_level(Num_Sample);
				if(++j == Num_Sample)
				{		
					printf("THD = %0.2f\r\n",THD);
					//串口屏显示图像		
					//显示波形
					printf2("cle s0.id,0\xff\xff\xff");
					for(i = 0; i < FFT_LENGTH/2; i++)
					{
						printf2("add s0.id,0,%d\xff\xff\xff", (uint8_t)(adc_buff[i]* 3.3 / 4095  *77.27));		
					}	
					delay_ms(2);	
					//发送THD值
					printf2f("t1.txt=\"%0.2f\"\xff\xff\xff",THD);
					
					//让其循环
					THD = 0;
					j = 0;
				}	
		}
//拓展部分1
		else if(Page == 2)
		{
//初始化
				Page_Before	= Page;	
//按键操作

////等待电路稳定
//				delay_ms(50);
//采集电压	
				MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);
				MAP_TimerEnable(TIMER0_BASE, TIMER_A);
				while(!AdcConvEnd);
				AdcConvEnd = false;
				MAP_uDMAChannelDisable(UDMA_CH16_ADC0_2);
				MAP_TimerDisable(TIMER0_BASE, TIMER_A);
//卡尔曼滤波
				for(i = 0; i < FFT_LENGTH; i++)
				{
					adc_buff[i] = KalmanFilter(&kfp,adc_buff[i]);
				}	
//直接输出			
				for(i = 0; i < FFT_LENGTH; i++)
				{
					fft_inputbuf[i*2] = adc_buff[i] * 3.3 / 4095;
					fft_inputbuf[i*2 + 1] = 0;
				}		
//FFT计算
				arm_cfft_radix4_f32(&scfft,fft_inputbuf);	//FFT计算（基4）				
				arm_cmplx_mag_f32(fft_inputbuf,fft_outputbuf,FFT_LENGTH);	//把运算结果复数求模得幅值 				
//THD										
				THD_and_level(Num_Sample);
				if(++j == Num_Sample)
				{		
					printf("THD = %0.2f\r\n",THD);
					//发送THD值					
					switch(Type_Auto)
					{
						case 0:printf2f("t6.txt=\"%0.2f\"\xff\xff\xff",THD);break;
						case 1:printf2f("t7.txt=\"%0.2f\"\xff\xff\xff",THD);break;
						case 2:printf2f("t8.txt=\"%0.2f\"\xff\xff\xff",THD);break;
						case 3:printf2f("t9.txt=\"%0.2f\"\xff\xff\xff",THD);break;
						case 4:printf2f("t5.txt=\"%0.2f\"\xff\xff\xff",THD);break;
						default:printf("Type_Auto error\r\n");
					}	
					
					//让其循环
					if(Type_Auto++ == 4) Type_Auto = 0;
					K1_ON();
					K2_ON();
					switch(Type_Auto)
					{
						case 0: Type_Wu();break;
						case 1: Type_Ding();break;
						case 2: Type_Di();break;
						case 3: Type_Shuang();break;
						case 4: Type_Jiao();break;
						default:printf("Type error\r\n");
					}
					delay_ms(50);

					THD = 0;
					j = 0;
				}	
		}
//拓展部分2
		else if(Page == 3)
		{
//初始化
				Page_Before	= Page;	
//按键操作
				K1_ON();
				K2_ON();
				switch(Type)
				{
					case 0: Type_Wu();break;
					case 1: Type_Ding();break;
					case 2: Type_Di();break;
					case 3: Type_Shuang();break;
					case 4: Type_Jiao();break;
					default:printf("Type error\r\n");
				}
//动态调整采样率
				if(j == 0)
				{
					fin=0;
					f_get_flag=false;	
					MAP_TimerEnable(TIMER3_BASE, TIMER_B);
					MAP_TimerEnable(TIMER1_BASE, TIMER_A);
					while(!f_get_flag);
					MAP_TimerDisable(TIMER3_BASE, TIMER_B);
					MAP_TimerDisable(TIMER1_BASE, TIMER_A);
					Dynamically_Adjus(fin);		
				}				
//采集电压	
				MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);
				MAP_TimerEnable(TIMER0_BASE, TIMER_A);
				while(!AdcConvEnd);
				AdcConvEnd = false;
				MAP_uDMAChannelDisable(UDMA_CH16_ADC0_2);
				MAP_TimerDisable(TIMER0_BASE, TIMER_A);
//卡尔曼滤波
				for(i = 0; i < FFT_LENGTH; i++)
				{
					adc_buff[i] = KalmanFilter(&kfp,adc_buff[i]);
				}
//直接输出			
				for(i = 0; i < FFT_LENGTH; i++)
				{
					fft_inputbuf[i*2] = adc_buff[i] * 3.3 / 4095;
					fft_inputbuf[i*2 + 1] = 0;
				}		
//FFT计算
				arm_cfft_radix4_f32(&scfft,fft_inputbuf);	//FFT计算（基4）				
				arm_cmplx_mag_f32(fft_inputbuf,fft_outputbuf,FFT_LENGTH);	//把运算结果复数求模得幅值 				
//THD							
				THD_and_level(Num_Sample);
				if(++j == Num_Sample)
				{		
					printf("THD = %0.2f\r\n",THD);
					//发送THD值
					printf2f("t1.txt=\"%0.2f\"\xff\xff\xff",THD);
					
					//让其循环
					THD = 0;
					j = 0;
//					delay_ms(50);	
				}
		}				
	}
}

/* Interrupt ------------------------------------------------------------------*/
void ADC0SS2_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

    /* If the interrupt status for Sequencer-2 is set the
     * clear the status and read the data */
    if((getIntStatus & ADC_INT_DMA_SS2) == ADC_INT_DMA_SS2)
    {
        /* Clear the ADC interrupt flag. */
        MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);

        /* Reconfigure the channel control structure and enable the channel */
        MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                                   UDMA_MODE_BASIC,
                                   (void *)&ADC0->SSFIFO2, (void *)&adc_buff,
                                   sizeof(adc_buff)/2);

        MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);

        /* Set conversion flag to true */
        AdcConvEnd = true;
    }
}

//串口屏接受中断
void UART2_IRQHandler(void)
{
	uint32_t ui32Status;
	
	ui32Status = MAP_UARTIntStatus(UART2_BASE, true);
	MAP_UARTIntClear(UART2_BASE, ui32Status);
	while(MAP_UARTCharsAvail(UART2_BASE))
	{
		ReceiveData[0] = MAP_UARTCharGetNonBlocking(UART2_BASE);//HWREG(UART2_BASE + 0x00000000);
		MAP_UARTCharPutNonBlocking(UART2_BASE, MAP_UARTCharGetNonBlocking(UART2_BASE));
//代码区
//		printf("ReceiveData = %d\r\n", ReceiveData[0]);
		switch(ReceiveData[0]-97)
		{
			case  0: Page =  0;printf("Page0\r\n");break;
			case  1: Page =  1;printf("Page1\r\n");break;
			case  2: Page =  2;printf("Page2\r\n");break;
			case  3: K1	  =  1;printf("K1_ON\r\n");break;
			case  4: K1	  =  2;printf("K1_OFF\r\n");break;
			case  5: K2	  =  1;printf("K2_ON\r\n");break;
			case  6: K2	  =  2;printf("K2_OFF\r\n");break;
			case  7: Type =  1;printf("Type_Wu\r\n");break;
			case  8: Type =  2;printf("Type_Ding\r\n");break;
			case  9: Type =  3;printf("Type_Di\r\n");break;
			case 10: Type =  4;printf("Type_Shuang\r\n");break;
			case 11: Type =  5;printf("Type_Jiao\r\n");break;
			case 12: Page =  3;printf("Page3\r\n");break;
			default:printf("ReceiveData error\r\n");
		}
	}
}

void TIMER1A_IRQHandler(void)
{
	MAP_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	printf("fin = %d\r\n",fin);
	f_get_flag = true;
}

void TIMER3B_IRQHandler(void)
{
	uint32_t getTimerIntStatus;
	getTimerIntStatus = MAP_TimerIntStatus(TIMER3_BASE, true);
	MAP_TimerIntClear(TIMER3_BASE, getTimerIntStatus);
//代码区
	fin++;
}
/* Function ------------------------------------------------------------------*/
void K1_ON(void)
{
	GPIOPinWrite(Switch1_GPIO, Switch1_PIN, 0x00);
}
void K1_OFF(void)
{
	GPIOPinWrite(Switch1_GPIO, Switch1_PIN, Switch1_PIN);	
}
void K2_ON(void)
{
	GPIOPinWrite(Switch2_GPIO, Switch2_PIN, 0x00);
}
void K2_OFF(void)
{
	GPIOPinWrite(Switch2_GPIO, Switch2_PIN, Switch2_PIN);
}
void Type_Wu(void)
{
	GPIOPinWrite(Switch3_GPIO, Switch3_PIN, Switch3_PIN);
	GPIOPinWrite(Switch4_GPIO, Switch4_PIN, Switch4_PIN);
	GPIOPinWrite(Switch5_GPIO, Switch5_PIN, 0x00);
	GPIOPinWrite(Switch6_GPIO, Switch6_PIN, 0x00);
	GPIOPinWrite(Switch7_GPIO, Switch7_PIN, Switch7_PIN);
}
void Type_Ding(void)
{
	GPIOPinWrite(Switch3_GPIO, Switch3_PIN, 0x00);
	GPIOPinWrite(Switch4_GPIO, Switch4_PIN, Switch4_PIN);
	GPIOPinWrite(Switch5_GPIO, Switch5_PIN, Switch5_PIN);
	GPIOPinWrite(Switch6_GPIO, Switch6_PIN, 0x00);
	GPIOPinWrite(Switch7_GPIO, Switch7_PIN, Switch7_PIN);
}
void Type_Di(void)
{
	GPIOPinWrite(Switch3_GPIO, Switch3_PIN, 0x00);
	GPIOPinWrite(Switch4_GPIO, Switch4_PIN, 0x00);
	GPIOPinWrite(Switch5_GPIO, Switch5_PIN, Switch5_PIN);
	GPIOPinWrite(Switch6_GPIO, Switch6_PIN, 0x00);
	GPIOPinWrite(Switch7_GPIO, Switch7_PIN, Switch7_PIN);
}
void Type_Shuang(void)
{
	GPIOPinWrite(Switch3_GPIO, Switch3_PIN, Switch3_PIN);
	GPIOPinWrite(Switch4_GPIO, Switch4_PIN, Switch4_PIN);
	GPIOPinWrite(Switch5_GPIO, Switch5_PIN, Switch5_PIN);
	GPIOPinWrite(Switch6_GPIO, Switch6_PIN, 0x00);
	GPIOPinWrite(Switch7_GPIO, Switch7_PIN, Switch7_PIN);
}
void Type_Jiao(void)
{
	GPIOPinWrite(Switch3_GPIO, Switch3_PIN, Switch3_PIN);
	GPIOPinWrite(Switch4_GPIO, Switch4_PIN, Switch4_PIN);
	GPIOPinWrite(Switch5_GPIO, Switch5_PIN, 0x00);
	GPIOPinWrite(Switch6_GPIO, Switch6_PIN, Switch6_PIN);
	GPIOPinWrite(Switch7_GPIO, Switch7_PIN, 0x00);
}

void THD_and_level(uint8_t NUM)
{
	float thd_fz=0,thd_fm=0;

	//找出主频率点的下表 i
	for(i=1;i<FFT_LENGTH/2;i++)
		if(fft_outputbuf[i]>30) break;  
	
	thd_fm = fft_outputbuf[i];
	thd_fz = sqrt(pow(fft_outputbuf[i*2],2)+pow(fft_outputbuf[i*3],2)+pow(fft_outputbuf[i*4],2)+pow(fft_outputbuf[i*5],2));
//	thd_fm = sqrt((pow(fft_outputbuf[i-2],2)+pow(fft_outputbuf[i-1],2)+pow(fft_outputbuf[i],2)+pow(fft_outputbuf[i+1],2)+pow(fft_outputbuf[i+2],2)));
//	thd_fz = sqrt(
//	(pow(fft_outputbuf[i*2-2],2)+pow(fft_outputbuf[i*2-1],2)+pow(fft_outputbuf[i*2],2)+pow(fft_outputbuf[i*2+1],2)+pow(fft_outputbuf[i*2+2],2))+
//	(pow(fft_outputbuf[i*3-2],2)+pow(fft_outputbuf[i*3-1],2)+pow(fft_outputbuf[i*3],2)+pow(fft_outputbuf[i*3+1],2)+pow(fft_outputbuf[i*3+2],2))+
//	(pow(fft_outputbuf[i*4-2],2)+pow(fft_outputbuf[i*4-1],2)+pow(fft_outputbuf[i*4],2)+pow(fft_outputbuf[i*4+1],2)+pow(fft_outputbuf[i*4+2],2))+
//	(pow(fft_outputbuf[i*5-2],2)+pow(fft_outputbuf[i*5-1],2)+pow(fft_outputbuf[i*5],2)+pow(fft_outputbuf[i*5+1],2)+pow(fft_outputbuf[i*5+2],2)));
	
	THD += thd_fz/thd_fm*100 / NUM;
}

void THD_and_level_withCompensate(uint8_t NUM)
{
	float thd_fz=0,thd_fm=0;

	//找出主频率点的下表 i
	for(i=1;i<FFT_LENGTH/2;i++)
	{
		if(fft_outputbuf[i]>300) break;  	
	}
	
	thd_fm = sqrt
	(pow(fft_outputbuf[i-4],2)+pow(fft_outputbuf[i-3],2)+pow(fft_outputbuf[i-2],2)+pow(fft_outputbuf[i-1],2)+
	 pow(fft_outputbuf[i],2)+
	 pow(fft_outputbuf[i+4],2)+pow(fft_outputbuf[i+3],2)+pow(fft_outputbuf[i+2],2)+pow(fft_outputbuf[i+1],2));
							
	thd_fz = sqrt
	(pow(fft_outputbuf[i*2-4],2)+pow(fft_outputbuf[i*2-3],2)+pow(fft_outputbuf[i*2-2],2)+pow(fft_outputbuf[i*2-1],2)+
	 pow(fft_outputbuf[i*2],2)+
	 pow(fft_outputbuf[i*2+4],2)+pow(fft_outputbuf[i*2+3],2)+pow(fft_outputbuf[i*2+2],2)+pow(fft_outputbuf[i*2+1],2)+
	
	 pow(fft_outputbuf[i*3-4],2)+pow(fft_outputbuf[i*3-3],2)+pow(fft_outputbuf[i*3-2],2)+pow(fft_outputbuf[i*3-1],2)+
	 pow(fft_outputbuf[i*3],2)+
	 pow(fft_outputbuf[i*3+4],2)+pow(fft_outputbuf[i*3+3],2)+pow(fft_outputbuf[i*3+2],2)+pow(fft_outputbuf[i*3+1],2)+
	 
	 pow(fft_outputbuf[i*4-4],2)+pow(fft_outputbuf[i*4-3],2)+pow(fft_outputbuf[i*4-2],2)+pow(fft_outputbuf[i*4-1],2)+
	 pow(fft_outputbuf[i*4],2)+
	 pow(fft_outputbuf[i*4+4],2)+pow(fft_outputbuf[i*4+3],2)+pow(fft_outputbuf[i*4+2],2)+pow(fft_outputbuf[i*4+1],2)+
	 
	 pow(fft_outputbuf[i*5-4],2)+pow(fft_outputbuf[i*5-3],2)+pow(fft_outputbuf[i*5-2],2)+pow(fft_outputbuf[i*5-1],2)+
	 pow(fft_outputbuf[i*5],2)+
	 pow(fft_outputbuf[i*5+4],2)+pow(fft_outputbuf[i*5+3],2)+pow(fft_outputbuf[i*5+2],2)+pow(fft_outputbuf[i*5+1],2));
	
	 THD += thd_fz/thd_fm*100 / NUM;
}

void Dynamically_Adjus(uint32_t f)
{
	delay_ms(2);		
	printf("fadc = %d\r\n",(uint32_t)(102.4 * f));
	if(fadc > 2000001) MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, (ui32SysClock/2000001));
	else		 					 MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, (ui32SysClock/(uint32_t)(102.4 * f)));
}

void Switch_GPIO_Config()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
	{
	}
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
	{
	}
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
	{
	}
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
	{
	}
	GPIOPinTypeGPIOOutput(Switch1_GPIO, Switch1_PIN);
	GPIOPinTypeGPIOOutput(Switch2_GPIO, Switch2_PIN);
	GPIOPinTypeGPIOOutput(Switch3_GPIO, Switch3_PIN);
	GPIOPinTypeGPIOOutput(Switch4_GPIO, Switch4_PIN);
	GPIOPinTypeGPIOOutput(Switch5_GPIO, Switch5_PIN);
	GPIOPinTypeGPIOOutput(Switch6_GPIO, Switch6_PIN);
	GPIOPinTypeGPIOOutput(Switch7_GPIO, Switch7_PIN);
}
	
	