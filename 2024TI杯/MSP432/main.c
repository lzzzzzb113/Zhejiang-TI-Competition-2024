/*
 * CMSIS DSP Function Used:
 * 1. arm_sqrt_f32
 * 2. arm_cfft_q15
 * 3. arm_cmplx_mag_q15
 *                MSP432E401Y
 *             ------------------
 *         /|\|                  |
 *          | |               PE0|<--AIN3
 *          --|RST            PE3|<--AIN0
 *            |                  |
 *            |                  |
 *            |                  |
 *            |               PA0|<--U0RX
 *            |               PA1|-->U0TX
 *******************************************************************************/
/* DriverLib Includes */
#include <driverlib.h>

/* Standard Includes */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Display Include via console */
#include "uartstdio.h"

//#include "arm_const_structs.h"
//#include "arm_math.h"

/* LCD Includes */
#include "lcd.h"
#include "lcd_init.h"

/* ADC Includes */
#include "bsp_ADC.h"

/* =1启用调试 */
#define START_Test 0

float wavebuff[5];

volatile bool setBufAReady = false;

volatile bool ChangeCHflag = false;
volatile bool ChangeCHTo10 = false;
// volatile bool LastChangeCHTo10 = false;

uint16_t tem_THD;
uint8_t thdbuff_i;
uint16_t THDbuffer[11];
uint16_t THDx;
uint32_t set_frq;

/* Global variables for RMS and DC calculation */
volatile float rmsBuff;
volatile float dcBuff;
float rmsCalculation;

volatile float DispPeriod, LastDispPeriod;
volatile float uiFreq;

/* f32 fft data */
//arm_rfft_fast_instance_f32 S;
float testInput_fft_1024[1024] = {0};
float testOutput_fft_1024[1024] = {0};

/* [0] = 基波 [2] = 二阶谐波 */
float Maxfft_Output[5];

void Calculate_Channl0_FFTValue(void);
void bubble_sort(float arr[], int len);

volatile uint8_t TxBuffer[7] = {0xEF, 0X3A};

void ADC0SS2_IRQHandler(void)
{
	uint32_t getIntStatus;

	/* Get the interrupt status from the ADC */
	getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

	/* If the interrupt status for Sequencer-2 is set the
	 * clear the status and read the data */
	if ((getIntStatus & ADC_INT_DMA_SS2) == ADC_INT_DMA_SS2)
	{
		/* Clear the ADC interrupt flag. */
		MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);

		/* Reconfigure the channel control structure and enable the channel */
		MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
								   UDMA_MODE_BASIC,
								   (void *)&ADC0->SSFIFO2, (void *)&dstBufferA,
								   sizeof(dstBufferA) / 4);

		MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);

		/* Set conversion flag to true */
		setBufAReady = true;
	}
}

/*  N值可根据实际情况调整
23.    排序采用冒泡法*/
#define N 11

uint16_t Mid_filter(void)
{
	uint8_t i, j, count;
	uint16_t temp, sum;

	for (i = 0; i < N - 1; i++)
		for (j = 0; j < N - 1 - i; j++)
			if (THDbuffer[j] > THDbuffer[j + 1])
			{
				temp = THDbuffer[j];
				THDbuffer[j] = THDbuffer[j + 1];
				THDbuffer[j + 1] = temp;
			}
	sum = 0;
	for (count = 1; count < N - 1; count++)
		sum += THDbuffer[count];
	return (uint16_t)(sum / (N - 2));
}

int main(void)
{
	uint32_t systemClock;

	/* Configure the system clock for 120 MHz */
	// systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
	systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 1000000);

	/* Initialize serial console */
	ConfigureUART(systemClock);

	/* Initialize fft parameter */
//	arm_rfft_fast_init_f32(&S, NUM_SAMPLES);

	/* 初始化ADC0 */
	ConfigureADC0(systemClock);

	/* 初始化LCD */
	LCD_Init();

	while (1)
	{
		/* 若通道采集完成，处理ADC数据 */
		if (setBufAReady == true)
		{
			/* 通道1采集完毕，关闭通道1采集 */
			MAP_TimerDisable(TIMER0_BASE, TIMER_A);

			/* 再清位 */
			setBufAReady = false;

			/* 计算fft */
			Calculate_Channl0_FFTValue();

			/* 若要切换通道 */
			if (ChangeCHflag == true)
			{
				/* clear bit */
				ChangeCHflag = false;

				/* 切换通道到10倍放大 */
				if (ChangeCHTo10 == true)
				{
					/* A sample sequence must be disabled before it is configured. */
					ADCSequenceDisable(ADC0_BASE, 2);

					/* Configure Sequencer 2 to sample the analog channel : AIN0. The end of
					 * conversion and interrupt generation is set for AIN0 */
					MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);

					/* Since sample sequence 2 is now configured, it must be enabled. */
					MAP_ADCSequenceEnable(ADC0_BASE, 2);
				}
				else /* 切换通道到2倍放大 */
				{
					/* A sample sequence must be disabled before it is configured. */
					ADCSequenceDisable(ADC0_BASE, 2);

					/* Configure Sequencer 2 to sample the analog channel : AIN0. The end of
					 * conversion and interrupt generation is set for AIN0 */
					MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

					/* Since sample sequence 2 is now configured, it must be enabled. */
					MAP_ADCSequenceEnable(ADC0_BASE, 2);
				}
			}

			/* 数据处理完，开启通道ADC采集 */
			MAP_TimerEnable(TIMER0_BASE, TIMER_A);

			/* 修改量程 */
			if (ChangeSampFreq == 1)
			{
				ChangeSampFreq = 0;
				MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, (systemClock / sampfreq)); // czk
			}
		}
	}
}

void Calc_THD(float *ptr_buf_in, uint16_t len, uint16_t *ptr_THD, uint32_t *ptr_S_Frq)
{
	float Tab_Harmonic[64][3];
	uint16_t scan_frq_cnt = 2;

	uint16_t setFFTmaxFreqIndex;
	float setFFTmaxValue;
	uint16_t i;
	// 谐波中心
	float tem_frq;
	uint16_t index_start, index_end;
	uint16_t rcd_har_pos;
	float tem_float;
	uint16_t tem_max;

	float THDx;
	int THD;

	// 获得最大的幅值和所在的位置
	setFFTmaxFreqIndex = 0;
	setFFTmaxValue = 0;
	for (i = 1; i < 512; i++)
	{
		if (setFFTmaxValue < ptr_buf_in[i])
		{
			setFFTmaxValue = ptr_buf_in[i];
			setFFTmaxFreqIndex = i;
		}
	}
	//
	for (i = 0; i < 16; i++)
	{
		Tab_Harmonic[i][0] = 0;
		Tab_Harmonic[i][1] = 0;
		Tab_Harmonic[i][2] = 0;
	}

	// 查找基波和谐波点
	Tab_Harmonic[0][0] = (float)setFFTmaxFreqIndex;
	Tab_Harmonic[0][1] = (float)setFFTmaxFreqIndex * sampfreq / NUM_SAMPLES;
	Tab_Harmonic[0][2] = ptr_buf_in[setFFTmaxFreqIndex];
	rcd_har_pos = 1;
	scan_frq_cnt = 2;
	while (1)
	{

		tem_frq = scan_frq_cnt * Tab_Harmonic[0][0];
		scan_frq_cnt++;

		if (tem_frq > 512)
		{
			break;
		}

		if (tem_frq > (setFFTmaxFreqIndex / 4.0))
		{
			index_start = (uint16_t)(tem_frq - (setFFTmaxFreqIndex / 4.0));
		}
		else
		{
			index_start = 0;
		}

		if ((tem_frq + (setFFTmaxFreqIndex / 4.0)) > 512)
		{
			index_end = 512;
		}
		else
		{
			index_end = (uint16_t)(tem_frq + (setFFTmaxFreqIndex / 4));
		}
		// find max
		tem_float = 0;
		for (i = index_start; i < index_end; i++)
		{
			if (ptr_buf_in[i] > tem_float)
			{
				tem_float = ptr_buf_in[i];
				tem_max = i;
			}
		}

		Tab_Harmonic[rcd_har_pos][0] = (float)(tem_max);
		Tab_Harmonic[rcd_har_pos][1] = (float)Tab_Harmonic[rcd_har_pos][0] * sampfreq / NUM_SAMPLES;
		Tab_Harmonic[rcd_har_pos][2] = ptr_buf_in[tem_max];
		rcd_har_pos++;

		if (rcd_har_pos > 16)
		{
			break;
		}
	}

	for (i = 0; i < 16; i++)
	{
		int tem_int1, tem_int2, tem_int3;

		tem_int1 = Tab_Harmonic[i][0];
		tem_int2 = Tab_Harmonic[i][1];
		tem_int3 = Tab_Harmonic[i][2] * 1000;
	}
	
	tem_float = 0;
	for (i = 1; i < 5; i++)
	{
		tem_float = tem_float + Tab_Harmonic[i][2] * Tab_Harmonic[i][2];
	}

	THDx = sqrt(tem_float) / Tab_Harmonic[0][2];

	THD = THDx * 1000;

	*ptr_THD = THD;
	*ptr_S_Frq = (uint32_t)Tab_Harmonic[0][1];

	JiBoVal = Tab_Harmonic[1][2] / Tab_Harmonic[0][2];
	XieBoVal = Tab_Harmonic[2][2] / Tab_Harmonic[0][2];
	XieBoVal3 = Tab_Harmonic[3][2] / Tab_Harmonic[0][2];
	XieBoVal4 = Tab_Harmonic[4][2] / Tab_Harmonic[0][2];
	XieBoVal5 = Tab_Harmonic[5][2] / Tab_Harmonic[0][2];
}

void send(void);

char text[50];
uint8_t putvalueflag;

uint8_t Mode_Select_Amp = 8;
void Calculate_Channl0_FFTValue(void)
{
	uint32_t i;
	uint32_t setFFTmaxValue;
	uint32_t setFFTmaxFreqIndex;
	uint32_t MaxInputValue, MinInputValue;

	/* 执行1024点浮点FFT运算 */
	for (i = 0; i < NUM_SAMPLES; i++)
	{
		testInput_fft_1024[i] = dstBufferA[i];
	}

	/* 找到输入数据的最大值与最小值 */
	MaxInputValue = dstBufferA[0];
	MinInputValue = dstBufferA[0];

	for (i = 1; i < NUM_SAMPLES; i++)
	{
		if (dstBufferA[i] > MaxInputValue)
		{
			MaxInputValue = dstBufferA[i];
		}
		else if (dstBufferA[i] < MinInputValue)
		{
			MinInputValue = dstBufferA[i];
		}
	}

	/* 420对应输入伏值在0-180mv 左右 */
	if ((MaxInputValue - MinInputValue) < 800)
	{
		/* 切换到放大10倍的通道  */
		ChangeCHflag = true;
		ChangeCHTo10 = true;
	}
	else
	{
		ChangeCHflag = true;
		ChangeCHTo10 = false;
	}

	/* 1024点实序列快速FFT, 浮点 */

	for (i = 0; i < NUM_SAMPLES; i++)
	{
		testInput_fft_1024[i] = testInput_fft_1024[i] * 3.3f / 4096;
	}

//	arm_rfft_fast_f32(&S, testInput_fft_1024, testOutput_fft_1024, 0);

	/* 求1024点的幅值 */
//	arm_cmplx_mag_f32(testOutput_fft_1024, testInput_fft_1024, 1024);

	/* 获得最大的幅值和所在的位置 */
	setFFTmaxFreqIndex = 0;
	setFFTmaxValue = 0;
	for (i = 1; i < 512; i++)
	{
		if (setFFTmaxValue < testInput_fft_1024[i])
		{
			setFFTmaxValue = testInput_fft_1024[i];
			setFFTmaxFreqIndex = i;
		}
	}

	Calc_THD(testInput_fft_1024, 512, &tem_THD, &set_frq);

	//	if( ( Mode_Select_Amp % 10 ) == 8 )					// 取频率
	//	{
	//		if( set_frq < 100000 )
	//		{
	//			if( ( set_frq % 1000 ) > 500 )
	//			{
	//				set_frq = ( set_frq / 1000 + 1 ) * 1000 ;
	//			}
	//			else
	//			{
	//				set_frq = ( set_frq / 1000 + 0 ) * 1000 ;
	//			}
	//		}
	//		else if(  set_frq < 1000000 )
	//		{
	//			if( ( set_frq % 10000 ) > 5000 )
	//			{
	//				set_frq = ( set_frq / 100000 + 1 ) * 100000 ;
	//			}
	//			else
	//			{
	//				set_frq = ( set_frq / 100000 + 0 ) * 100000 ;
	//			}
	//		}
	//		else if(  set_frq < 10000000 )
	//		{
	//			if( ( set_frq % 100000 ) > 50000 )
	//			{
	//				set_frq = ( set_frq / 1000000 + 1 ) * 1000000 ;
	//			}
	//			else
	//			{
	//				set_frq = ( set_frq / 1000000 + 0 ) * 1000000 ;
	//			}
	//		}
	//
	//		sampfreq = set_frq * 8 ;
	//		Mode_Select_Amp += 10 ;
	//
	//		if( Mode_Select_Amp == 58 )
	//		{
	//			Mode_Select_Amp = 7;
	//		}
	//
	//		ChangeSampFreq = 1;
	//		UARTprintf("THD_8: %d  FRQ:%d , set_FRQ:%d\n" , tem_THD  , set_frq , sampfreq );
	//	}
	//	else if( Mode_Select_Amp % 10 == 7 )					// 取THD
	//	{
	if (set_frq < 100000)
	{
		if ((set_frq % 1000) > 500)
		{
			set_frq = (set_frq / 1000 + 1) * 1000;
		}
		else
		{
			set_frq = (set_frq / 1000 + 0) * 1000;
		}
	}
	else if (set_frq < 1000000)
	{
		if ((set_frq % 10000) > 5000)
		{
			set_frq = (set_frq / 100000 + 1) * 100000;
		}
		else
		{
			set_frq = (set_frq / 100000 + 0) * 100000;
		}
	}
	else if (set_frq < 10000000)
	{
		if ((set_frq % 100000) > 50000)
		{
			set_frq = (set_frq / 1000000 + 1) * 1000000;
		}
		else
		{
			set_frq = (set_frq / 1000000 + 0) * 1000000;
		}
	}
	sampfreq = set_frq * 20;

	if (sampfreq < 19000)
	{
		sampfreq = 2000000;
	}
	else if (sampfreq < 20000)
	{
		sampfreq = 20000;
	}

	if (sampfreq > 2000000)
	{
		sampfreq = 2000000;
	}

	ChangeSampFreq = 1;

	THDbuffer[thdbuff_i] = tem_THD;
	thdbuff_i++;

	if (thdbuff_i == 11)
	{
		thdbuff_i = 0;
		THDx = Mid_filter();
	}
	/* 估算频率 */
	uiFreq = (float)setFFTmaxFreqIndex * set_frq / NUM_SAMPLES;

	DispPeriod = set_frq / uiFreq;

	TxBuffer[2] = THDx / 10;
	TxBuffer[3] = THDx % 10;
	TxBuffer[5] = XieBoVal * 100;

	TxBuffer[4] = JiBoVal * 100;
	TxBuffer[6] = DispPeriod;

	Draw_Wave(&dstBufferA[10], DispPeriod);

	send();

	LastDispPeriod = DispPeriod;

	setFFTmaxValue = 0;
}

void send(void)
{
	int i;
	/* 发送帧头 */
	for (i = 0; i < 7; i++)
	{
		UARTCharPutNonBlocking(UART0_BASE, TxBuffer[i]);
		delay_ms(35);
	}

	for (i = 0; i < TxBuffer[6]; i++)
	{
		UARTCharPutNonBlocking(UART0_BASE, sendadcbuff[i]);
		delay_ms(35);
	}
}
