// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
	SYSCTL_RCGCGPIO_R |= 0x08;														//activate Port D clock
	int volatile delay = SYSCTL_RCGCGPIO_R;  						
	
	/*
	GPIO_PORTF_DIR_R |= 0x04;															//PF 2 heartbeat
	GPIO_PORTF_DEN_R |= 0x04;
	GPIO_PORTF_LOCK_R	= 0x4C4F434B;
	GPIO_PORTF_CR_R	= 0x1F;
	*/
	
	GPIO_PORTD_DIR_R &= ~0x04;
	GPIO_PORTD_AFSEL_R |= 0x04;														//enable alternate funct
	GPIO_PORTD_DEN_R &= ~0x04;
	GPIO_PORTD_AMSEL_R |= 0x04;														//enable analog funct
	
	SYSCTL_RCGCADC_R |= 0x01;															//activate ADC0
	delay = SYSCTL_RCGCADC_R; 
	delay = SYSCTL_RCGCADC_R; 
	delay = SYSCTL_RCGCADC_R; 
	delay = SYSCTL_RCGCADC_R; 
	ADC0_PC_R = 0x01;																			//125k Hz sample rate
	ADC0_SSPRI_R = 0x0123;																//seq 3 highest priority @ 0
	ADC0_ACTSS_R &= ~0x0008;															//disable sample seq 3
	ADC0_EMUX_R &= ~0xF000;																//seq 3 software trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 5;			//set channel 5
	ADC0_SSCTL3_R = 0x0006;																//NO: TS0 D0 ,YES: IE0 END0
	ADC0_IM_R &= ~0x0008;																	//disable ss3 interrupts
	ADC0_ACTSS_R |= 0x0008;																//enable sample seq 3
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	uint32_t data;
	ADC0_PSSI_R = 0x0008; 
	while((ADC0_RIS_R & 0x08) == 0){};
	data = ADC0_SSFIFO3_R & 0x0FFF; 		//ADC conversion in FIFO buffer
	ADC0_ISC_R = 0x0008; 									//clear flag
  return data; 
}


