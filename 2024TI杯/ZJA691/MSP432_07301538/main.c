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
 *            |               PA0|<--U0RX
 *            |               PA1|-->U0TX
 *******************************************************************************/
/* DriverLib Includes */
#include <driverlib.h>

/* Display Include via console */
#include "uartstdio.h"

#include "FFTInc.h"
#include "SignalProcess.h"

/* LCD Includes */
#include "LCD1602.h"
#include "Key.h"

/* ADC Includes */
#include "bsp_ADC.h"

float wavebuff[5];

volatile bool setBufAReady = false;
volatile bool setBufBReady = false;

uint16_t tem_THD;
uint8_t thdbuff_i;
uint16_t THDx;
uint32_t set_frq;

/* Global variables for RMS and DC calculation */
volatile float rmsBuff;
volatile float dcBuff;
float rmsCalculation;

volatile float DispPeriod, LastDispPeriod;
volatile float uiFreq;

/* f32 fft data */
float testInput_fft_1024[NUM_SAMPLES] = {0};
float testOutput_fft_1024[NUM_SAMPLES] = {0};

/* [0] = 基波 [1] = 一次谐波 [2] = 二次谐波 */
float Maxfft_Output[11];

void Calc_Channl_FFTValue(void);

// nuedc 2024
uint8_t IO_A = 0, IO_B = 0; // IO反转用
void czk_GPIOPinWrite(uint32_t ui32Port, uint8_t ui8Pins, uint8_t ui8Val);
uint8_t ADC_Cnt = 0;

void ADC0SS2_IRQHandler(void)
{
	uint32_t getIntStatus;
	/* Get the interrupt status from the ADC */
	getIntStatus = ADCIntStatusEx(ADC0_BASE, true);

	/* If the interrupt status for Sequencer-2 is set the
	 * clear the status and read the data */
	if ((getIntStatus & ADC_INT_DMA_SS2) == ADC_INT_DMA_SS2)
	{
		/* Clear the ADC interrupt flag. */
		ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);

		/* Reconfigure the channel control structure and enable the channel */
		uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
							   UDMA_MODE_BASIC,
							   (void *)&ADC0->SSFIFO2, (void *)&dstBufferA,
							   sizeof(dstBufferA) / 4);
		uDMAChannelEnable(UDMA_CH16_ADC0_2);

		/* Set conversion flag to true */
		setBufAReady = true;
	}
}

void ADC1SS2_IRQHandler(void)
{
	uint32_t getIntStatus;
	/* Get the interrupt status from the ADC */
	getIntStatus = ADCIntStatusEx(ADC1_BASE, true);

	/* If the interrupt status for Sequencer-2 is set the
	 * clear the status and read the data */
	if ((getIntStatus & ADC_INT_DMA_SS2) == ADC_INT_DMA_SS2)
	{
		/* Clear the ADC interrupt flag. */
		ADCIntClearEx(ADC1_BASE, ADC_INT_DMA_SS2);

		/* Reconfigure the channel control structure and enable the channel */
		uDMAChannelTransferSet(UDMA_CH26_ADC1_2 | UDMA_PRI_SELECT,
							   UDMA_MODE_BASIC,
							   (void *)&ADC1->SSFIFO2, (void *)&dstBufferB,
							   sizeof(dstBufferB) / 4);
		uDMAChannelEnable(UDMA_CH26_ADC1_2);

		/* Set conversion flag to true */
		setBufBReady = true;
		IO_B = 1 - IO_B;
		czb_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, IO_B);
	}
}

void czb_GPIOPinWrite(uint32_t ui32Port, uint8_t ui8Pins, uint8_t ui8Val)
{
	ASSERT(_GPIOBaseValid(ui32Port));
	// 这一步就是帮你与好了对应的掩码。
	if (ui8Val > 0)
	{
		ui8Val = 0xff & ui8Pins;
	}
	GPIOPinWrite(ui32Port, ui8Pins, ui8Val);
}

int main(void)
{
	uint32_t systemClock;

	/* Configure the system clock for 120 MHz */
	systemClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

	/* Initialize serial console */
	ConfigureUART(systemClock);

	/* Enable the clock to GPIO Port E&F and wait for it to be ready */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while (!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
	{
	}

	/* 初始化ADC0 */
	ConfigureADC0(systemClock);
	/* 初始化ADC1 */
	ConfigureADC1(systemClock);

	TimerEnable(TIMER0_BASE, TIMER_A);

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2);

	/* 初始化LCD */
	Key_Init();
	LCD1602_REG_Init();
	LCD_Disp_Str(0x80, "Hello nuedc");
	Timer2_Init();
	test_update_epara();

	InitTableFFT(1024);

	while (1)
	{
		if (flag_Tmr_1ms != 0)
		{
			/* 若通道采集完成，处理ADC数据 */
			if (setBufAReady == true && setBufBReady == true)
			{
				TimerDisable(TIMER0_BASE, TIMER_A);
				/* 再清位 */
				setBufAReady = false;
				setBufBReady = false;

				/* 计算fft */
				Calc_Channl_FFTValue();
				if (ADC_Cnt < 4)
				{
					ADC_Cnt++;
				}

				/* A sample sequence must be disabled before it is configured. */
				ADCSequenceDisable(ADC0_BASE, 2);
				ADCSequenceDisable(ADC1_BASE, 2);

				/* Configure Sequencer 2 to sample the analog channel : AIN0. The end of
				 * conversion and interrupt generation is set for AIN0 */
				ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
				ADCSequenceStepConfigure(ADC1_BASE, 2, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);

				/* Since sample sequence 2 is now configured, it must be enabled. */
				ADCSequenceEnable(ADC0_BASE, 2);
				ADCSequenceEnable(ADC1_BASE, 2);

				TimerLoadSet(TIMER0_BASE, TIMER_A, (systemClock / SampleRate));

				/* 数据处理完，开启通道ADC采集 */
				TimerEnable(TIMER0_BASE, TIMER_A);
			}
			flag_Tmr_1ms = 0;
			GPIOL->DATA ^= BIT5;
			static uint16_t cnt_tmr_10ms = 0;

			cnt_tmr_10ms++;
			if (cnt_tmr_10ms > 10)
			{
				cnt_tmr_10ms = 0;
				Task_Key_Run();
			}

			static uint16_t cnt_tmr_500ms = 0;
			cnt_tmr_500ms++;
			if (cnt_tmr_500ms > 500)
			{
				cnt_tmr_500ms = 0;
				Task_LCD_Run();
			}
		}
	}
}
