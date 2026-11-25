#include "bsp_ADC.h"

/* DMA Buffer declaration and buffer complet flag */
uint32_t dstBufferA[NUM_SAMPLES];
uint32_t dstBufferB[NUM_SAMPLES];

/* The control table used by the uDMA controller.  This table must be aligned
 * to a 1024 byte boundary. */
#if defined(__ICCARM__)
#pragma data_alignment = 2048
uint8_t pui8ControlTable[2048];
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(pui8ControlTable, 2048)
uint8_t pui8ControlTable[2048];
#else
uint8_t pui8ControlTable[1024] __attribute__((aligned(1024)));
#endif

void ConfigureADC0(uint32_t systemClock)
{
	/* Configure PE3 as ADC input channel */
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
	
	/* Enable the clock to ADC-0 and wait for it to be ready */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	while (!(SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))) {
	}

	/* Configure Sequencer 2 to sample the analog channel : AIN0. The end of
	 * conversion and interrupt generation is set for AIN0 */
	ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END); //adc0基地址，序列2，排队0 通道0

	/* Enable sample sequence 2 with a timer signal trigger.  Sequencer 2
	 * will do a single sample when the timer generates a trigger on timeout*/
	ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_TIMER, 2);

	/* Clear the interrupt status flag before enabling. This is done to make
	 * sure the interrupt flag is cleared before we sample. */
	ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2); 
	ADCIntEnableEx(ADC0_BASE, ADC_INT_DMA_SS2);
	
	ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, 7);

	/* Enable the DMA request from ADC0 Sequencer 2 */
	ADCSequenceDMAEnable(ADC0_BASE, 2);

	/* Since sample sequence 2 is now configured, it must be enabled. */
	ADCSequenceEnable(ADC0_BASE, 2);

	/* Enable the Interrupt generation from the ADC-0 Sequencer */
	IntEnable(INT_ADC0SS2);

	/* Enable the DMA and Configure Channel for TIMER0A for Ping Pong mode of
	 * transfer */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	while (!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA))) {
	}

	uDMAEnable();

	/* Point at the control table to use for channel control structures. */
	uDMAControlBaseSet(pui8ControlTable); //这是一个1024的buff

	/* Map the ADC0 Sequencer 2 DMA channel */
	uDMAChannelAssign(UDMA_CH16_ADC0_2);

	/* Put the attributes in a known state for the uDMA ADC0 Sequencer 2
	 * channel. These should already be disabled by default. */
	uDMAChannelAttributeDisable(UDMA_CH16_ADC0_2,
			UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

	/* Configure the control parameters for the primary control structure for
	 * the ADC0 Sequencer 2 channel. The primary control structure is used for
	 * copying the data from ADC0 Sequencer 2 FIFO to dstBufferA. The transfer
	 * data size is 32 bits and the source address is not incremented while
	 * the destination address is incremented at 32-bit boundary.
	 */
	uDMAChannelControlSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
			UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_32 | UDMA_ARB_1);

	/* Set up the transfer parameters for the ADC0 Sequencer 2 primary control
	 * structure. The mode is Basic mode so it will run to completion. */
//    uDMAChannelTransferSet (UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,UDMA_MODE_PINGPONG,
//															 (void*)&ADC0->SSFIFO2, (void*)&dstBufferA,sizeof(dstBufferA)/4);
	uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
														 UDMA_MODE_BASIC,
														 (void *)&ADC0->SSFIFO2, (void *)&dstBufferA,
														 sizeof(dstBufferA)/4);

	/* The uDMA ADC0 Sequencer 2 channel is primed to start a transfer. As
	 * soon as the channel is enabled and the Timer will issue an ADC trigger,
	 * the ADC will perform the conversion and send a DMA Request. The data
	 * transfers will begin. */
	uDMAChannelEnable(UDMA_CH16_ADC0_2);

	/* Enable Timer-0 clock and configure the timer in periodic mode with
	 * a frequency of 1 KHz. Enable the ADC trigger generation from the
	 * timer-0. */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	while (!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))) {
	}

	TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, (systemClock / SampleRate)); //czk
	TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
	TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
}


void ConfigureADC1(uint32_t systemClock)
{
	/* Configure PE0 as ADC input channel */
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);

	/* Enable the clock to ADC-1 and wait for it to be ready */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
	while (!(SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1))) {
	}

	/* Configure Sequencer 2 to sample the analog channel : AIN0. The end of
	 * conversion and interrupt generation is set for AIN3 */
	                              /* adc1 基地址，序列2，排队0 通道3 */
	ADCSequenceStepConfigure(ADC1_BASE, 2, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END); 

	/* Enable sample sequence 2 with a timer signal trigger.  Sequencer 2
	 * will do a single sample when the timer generates a trigger on timeout*/
	ADCSequenceConfigure(ADC1_BASE, 2, ADC_TRIGGER_TIMER, 2);

	/* Clear the interrupt status flag before enabling. This is done to make
	 * sure the interrupt flag is cleared before we sample. */
	ADCIntClearEx(ADC1_BASE, ADC_INT_DMA_SS2); 	//CZK?
	ADCIntEnableEx(ADC1_BASE, ADC_INT_DMA_SS2);

	ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, 7);

	/* Enable the DMA request from ADC1 Sequencer 2 */
	ADCSequenceDMAEnable(ADC1_BASE, 2);

	/* Since sample sequence 2 is now configured, it must be enabled. */
	ADCSequenceEnable(ADC1_BASE, 2);

	/* Enable the Interrupt generation from the ADC-1 Sequencer */
	IntEnable(INT_ADC1SS2);

	/* Enable the DMA and Configure Channel for TIMER0A for Ping Pong mode of
	 * transfer */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	while (!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA))) {
	}

	uDMAEnable();

	/* Point at the control table to use for channel control structures. */
	uDMAControlBaseSet(pui8ControlTable); //这是一个1024的buff

	/* Map the ADC1 Sequencer 2 DMA channel */
	uDMAChannelAssign(UDMA_CH26_ADC1_2);

	/* Put the attributes in a known state for the uDMA ADC1 Sequencer 2
	 * channel. These should already be disabled by default. */
	uDMAChannelAttributeDisable(UDMA_CH26_ADC1_2,
			UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

	/* Configure the control parameters for the primary control structure for
	 * the ADC0 Sequencer 2 channel. The primary control structure is used for
	 * copying the data from ADC0 Sequencer 2 FIFO to dstBufferA. The transfer
	 * data size is 32 bits and the source address is not incremented while
	 * the destination address is incremented at 32-bit boundary.
	 */
	uDMAChannelControlSet(UDMA_CH26_ADC1_2 | UDMA_PRI_SELECT,
			UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_32 | UDMA_ARB_1);

	/* Set up the transfer parameters for the ADC0 Sequencer 2 primary control
	 * structure. The mode is Basic mode so it will run to completion. */
	uDMAChannelTransferSet(UDMA_CH26_ADC1_2 | UDMA_PRI_SELECT,
														 UDMA_MODE_BASIC,
														 (void *)&ADC1->SSFIFO1, (void *)&dstBufferB,
														 sizeof(dstBufferB)/4);

	/* The uDMA ADC0 Sequencer 2 channel is primed to start a transfer. As
	 * soon as the channel is enabled and the Timer will issue an ADC trigger,
	 * the ADC will perform the conversion and send a DMA Request. The data
	 * transfers will begin. */
	uDMAChannelEnable(UDMA_CH26_ADC1_2);

	/* Enable Timer-0 clock and configure the timer in periodic mode with
	 * a frequency of 1 KHz. Enable the ADC trigger generation from the
	 * timer-0. */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	while (!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1))) {
	}

	// TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC);
	// TimerLoadSet(TIMER1_BASE, TIMER_A, (systemClock / SampleRate)); //czk
	TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
	// TimerControlTrigger(TIMER1_BASE, TIMER_A, true);
}
