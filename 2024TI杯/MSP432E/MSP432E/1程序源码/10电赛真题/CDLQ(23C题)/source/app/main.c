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
#include "ADS112C04_i2c.h"
#include "ADS112C04.h"
#include "AD9910.h"
#include "math.h"
#include "uartstdio.h"
#include "tim.h"

/* Private define ------------------------------------------------------------*/
#define Num 10
#define PAI  3.1415926
#define Cl  0.000000022
#define Rl  2000 
 
/* Private variables ---------------------------------------------------------*/
uint32_t ui32SysClock;
uint16_t i,j;
uint8_t ReceiveData[3];
uint8_t HMI_Command = 0;

float ADC0_Value[10];
float ADC0_Value_Filter;

float ADC1_Value[10];
float ADC1_Value_Filter;

float ADC3_Value[10];
float ADC3_Value_Filter;

float Phase_Change = 0;
float Vrms_Change = 0;

int Vrms = 100;
long int f = 1000000;

float Vin,Vout,Phase;
double Cin,Din,Qin,Lin,Angel;
uint16_t Rin;
uint16_t Rr;

uint32_t PointA = 0,PointB = 1000000,PointC;
float VpointA,VpointB;
uint8_t f_flag = 0;

uint32_t fre = 1000000;
float ADC_Vol[2] = {0,-1};
/* Private function prototypes -----------------------------------------------*/
float Filter(float ADC1_Value[]); 
void bubble_sort(float ADC1_Value[]); 
void Correct_Phase(void);
void Correct_Vrms(void);

/* Main ----------------------------------------------------------------------*/
int main(void)
{
//初始化
	sys_init();
	uart0_init();
	uart2_init();
	ADS112_Init();
	Init_AD9910();
	Switch_Init();
	printf("\r\nBegin\r\n");
	
	while(1)
	{		
		if(HMI_Command == 1)//测量C与D
		{
			printf("\r\n");

//Range	
			ADS112_WriteReg0(ADS122C04_MUX_AIN1_AVSS,ADS122C04_GAIN_1,ADS122C04_PGA_DISABLED);
			for(i=0;i<Num;i++)
			{
				ADS112_Send_Command(ADS122C04_START_CMD);
				ADC1_Value[i] = ADS112_GetADC_Vol();	
				delay_ms(10);
			}			
			ADC1_Value_Filter = Filter(ADC1_Value);
			printf("RangeAD:%0.3f\r\n", ADC1_Value_Filter);
			delay_ms(5);

			
//SWITCH
			if(ADC1_Value_Filter > 0.13) 
			{
				Rr = 2200;
				Switch1_Off;
				printf("Switch1_Off\r\n");				
			}
			else
			{
				Rr = 20000;
				Switch1_On;
				printf("Switch1_On\r\n");
			}		
			
//DDS
			f = 10000;
			Vin = 100;
			AD9910_FreWrite(10000);
			AD9910_VrmsWrite(100);
			delay_ms(200);
			
//Vout			
			ADS112_WriteReg0(ADS122C04_MUX_AIN1_AVSS,ADS122C04_GAIN_1,ADS122C04_PGA_DISABLED);
			for(i=0;i<Num;i++)
			{
				ADS112_Send_Command(ADS122C04_START_CMD);
				ADC1_Value[i] = ADS112_GetADC_Vol();	
				delay_ms(10);
			}			
			Vout = Filter(ADC1_Value);
			printf("Vout:%0.3f\r\n", Vout);
			delay_ms(10);
			
//Phase			
			ADS112_WriteReg0(ADS122C04_MUX_AIN3_AVSS,ADS122C04_GAIN_1,ADS122C04_PGA_DISABLED);
			for(i=0;i<Num;i++)
			{
				ADS112_Send_Command(ADS122C04_START_CMD);
				ADC3_Value[i] = ADS112_GetADC_Vol();	
				delay_ms(10);
			}		
			ADC3_Value_Filter = Filter(ADC3_Value);
			Phase = -91.289*ADC3_Value_Filter + 174.98 + Phase_Change;
			printf("AD3:%0.5f\r\n",ADC3_Value_Filter);
			delay_ms(5);
			printf("Phase:%0.2f\r\n", Phase);
			delay_ms(5);
//			printf("tan(Phase*PAI/180):%0.2f\r\n", tan(Phase*PAI/180));
//			delay_ms(5);			
			
//Cin	
//			Cin = ((Vout*1000 / (Rr * Vin * 2 * PAI * f * sin(Phase*PAI/180)))*1000000000);
//			if(Cin < 10) printf("C:%0.3f\r\n",1.0074*Cin + 0.0886);
//			else printf("C:%0.3f\r\n",Cin);
//			delay_ms(5);

			Cin = ((Vout*1000 / (Rr * Vin * 2 * PAI * f * sin(Phase*PAI/180)))*1000000000);
			if(Cin < 10) Cin = 1.0074*Cin + 0.0886;
			printf("C:%0.3f\r\n",Cin);
			delay_ms(5);
			
//Din
//			Din = tan((Phase - 90.0)*PAI/180);
			Rin = 100;
			Din = fabsf(1/(tan((Phase)*PAI/180)))- 2*PAI*f*Cin*Rin*0.000000001 ;// 
//			printf("D:%0.3f\r\n",Din);
//			delay_ms(5);
			
			Angel = atan((1/Din))*180/PAI;
			printf("Angel:%0.3f\r\n",Angel);
			delay_ms(5);
			Din = 1/tan((Angel+0.6918)*PAI/180);
			printf("D:%0.3f\r\n",Din);  
			
			if(Cin < 100)
			{
				delay_ms(2);
				printf2f("t2.txt=\"%0.2f\"\xff\xff\xff",Cin);
				delay_ms(2);
				printf2f("t2.txt=\"%0.2f\"\xff\xff\xff",Cin);
				delay_ms(2);		
			}
			else
			{
				delay_ms(2);
				printf2f("t2.txt=\"%0.1f\"\xff\xff\xff",Cin);
				delay_ms(2);
				printf2f("t2.txt=\"%0.1f\"\xff\xff\xff",Cin);
				delay_ms(2);		
			}
			
			if(Din > 0)
			{
				printf2f("t6.txt=\"%0.3f\"\xff\xff\xff",Din);
				delay_ms(2);				
			}
			else
			{
				printf2f("t6.txt=\"%0.3f\"\xff\xff\xff",-Din);
				delay_ms(2);			
			}	
		
			Switch1_Off;
			HMI_Command = 0;
		}
		
		else if(HMI_Command == 2)//测量L与Q
		{
			printf("\r\n");
			
			f_flag = 0;
			fre = 1000000;						
			ADC_Vol[0] = 0;
			ADC_Vol[1] = -1;
			AD9910_VrmsWrite(100);
		  ADS112_WriteReg0(ADS122C04_MUX_AIN0_AVSS, ADS122C04_GAIN_1, ADS122C04_PGA_DISABLED);
			
			while(!f_flag)
			{
				AD9910_FreWrite(fre);
				delay_ms(2);
				ADS112_Send_Command(ADS122C04_START_CMD);
				ADC_Vol[0] = ADS112_GetADC_Vol();
				
				if(ADC_Vol[0] > ADC_Vol[1]) 
				{
					fre+= 10000;
					ADC_Vol[1] = ADC_Vol[0];
				}
				else 
				{
					printf("AD0:%0.3f\r\n",ADC_Vol[0]);
					f_flag = 1;
					fre -= 10000;
				}
			}
				
			delay_ms(5);	
			Lin = 100000000/(4*PAI*PAI*fre*fre*Cl);
			if(Lin > 10) Lin = Lin*1.374 - 2.6503;
			printf("L:%0.3f\r\n",Lin);

			delay_ms(5);
			Qin = 2.0 * PAI * (float)fre * Lin  / 5000000;
			printf("Q:%0.3f\r\n",Qin);
			
			if(Lin < 100)
			{
				delay_ms(2);
				printf2f("t2.txt=\"%0.2f\"\xff\xff\xff",Lin);
				delay_ms(2);
				printf2f("t2.txt=\"%0.2f\"\xff\xff\xff",Lin);
				delay_ms(2);		
			}
			else
			{
				delay_ms(2);
				printf2f("t2.txt=\"%0.1f\"\xff\xff\xff",Lin);
				delay_ms(2);
				printf2f("t2.txt=\"%0.1f\"\xff\xff\xff",Lin);
				delay_ms(2);			
			}
			
			printf2f("t6.txt=\"%0.2f\"\xff\xff\xff",Qin);
			delay_ms(2);			
							
			printf2f("t0.txt=\"%0.2fM\"\xff\xff\xff",(float)((float)fre/1000000));
			
			AD9910_FreWrite(0);
			
			HMI_Command = 0;
		}
		
		else if(HMI_Command == 3)//相位校准
		{	
			Correct_Phase();
			HMI_Command = 0;
		}
		
		else if(HMI_Command == 4)//电压校准
		{	
			Correct_Vrms();
			HMI_Command = 0;
		}
	}
}

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
		switch(ReceiveData[0]-48)
		{
			case  1: 
			HMI_Command = 1;
//			printf("C and D\r\n");
			break;
			
			case  2: 
			HMI_Command = 2;
//			printf("L and Q\r\n");
			break;
			
			case  3: 
			HMI_Command = 3;
//			printf("Correct_Phase\r\n");			
			break;
			
			case  4: 
			HMI_Command = 4;
//			printf("Correct_Vrms\r\n");			
			break;
		}
	}
}

void Correct_Vrms(void)
{
	ADS112_WriteReg0(ADS122C04_MUX_AIN1_AVSS,ADS122C04_GAIN_1,ADS122C04_PGA_DISABLED);
	for(i=0;i<Num;i++)
	{
		ADS112_Send_Command(ADS122C04_START_CMD);
		ADC1_Value[i] = ADS112_GetADC_Vol();	
		delay_ms(10);
	}
	ADC1_Value_Filter = Filter(ADC1_Value);
	Vrms_Change = 0.1 - (0.2196*(double)ADC1_Value_Filter - 0.0036);
	printf("Vrms_Change:%0.2f\r\n",Vrms_Change);	
}

void Correct_Phase(void)
{
	ADS112_WriteReg0(ADS122C04_MUX_AIN1_AVSS,ADS122C04_GAIN_1,ADS122C04_PGA_DISABLED);
	for(i=0;i<Num;i++)
	{
		ADS112_Send_Command(ADS122C04_START_CMD);
		ADC1_Value[i] = ADS112_GetADC_Vol();	
//			printf("1:%0.3f\r\n",ADC1_Value[i]);
		delay_ms(10);
	}		
	ADC1_Value_Filter = Filter(ADC1_Value);
//		printf("%0.5f\r\n",ADC1_Value_Filter);
	Phase_Change = 90 - (-91.289*ADC1_Value_Filter + 174.98);
	printf("Phase_Change:%0.2f\r\n",Phase_Change);	
}

float Filter(float ADC_Value[]) 
{
	float a = 0;
	bubble_sort(ADC_Value);
//	for(i=3;i<Num-3;i++)
//	{
//		printf("%d:%0.3f\r\n",i,ADC_Value[i]);
//		delay_ms(10);
//	}	
	for(i=3;i<Num-3;i++) a += ADC_Value[i]/4;
//	printf("a:%0.3f\r\n",a);
	return a;
}

void bubble_sort(float ADC_Value[]) 
{
	int a,b;
	float temp;
	for(a=0;a<Num;a++)
	{   
		for(b=0;b<Num-a-1;b++)
		{   
			if(ADC_Value[b] < ADC_Value[b+1])
			{   
				temp = ADC_Value[b];
				ADC_Value[b] = ADC_Value[b+1];
				ADC_Value[b+1] = temp;
			}   
		}       
	}   
}

