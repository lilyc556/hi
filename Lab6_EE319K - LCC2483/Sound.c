// Sound.c
// This module contains the SysTick ISR that plays sound
// Runs on LM4F120 or TM4C123
// Program written by: Lily Chieng, Ashiqa Momin
// Date Created: 3/6/17 
// Last Modified: 3/17/2020 
// Lab number: 6
// Hardware connections
// PE0-3 Piano key inputs, PB0-3 DAC Outputs, PF3 Heartbeat, PD3 Oscilloscope


// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "dac.h"
#include "../inc/tm4c123gh6pm.h"

const unsigned short wave[32]={8,9,11,12,13,14,14,15,15,15,14,14,13,12,11,9,8,7,5,4,3,2,2,1,1,1,2,2,3,4,5,7};
	
// **************Sound_Init*********************
// Initialize digital outputs and SysTick timer
// Called once, with sound/interrupts initially off
// Input: none
// Output: none
void Sound_Init(uint8_t whichNote){
	//initialize systick 
	//flag counts, indices
	//start with quiet
	//call dac init
  NVIC_ST_CTRL_R = 0x00;
	NVIC_ST_RELOAD_R = 0; //start quiet
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 0x07; 
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000; // priority of 1?
	DAC_Init();
}


// **************Sound_Play*********************
// Start sound output, and set Systick interrupt period 
// Sound continues until Sound_Play called again
// This function returns right away and sound is produced using a periodic interrupt
// Input: interrupt period
//           Units of period to be determined by YOU
//           Maximum period to be determined by YOU
//           Minimum period to be determined by YOU
//         if period equals zero, disable sound output
// Output: none
uint8_t time = 0;
void Sound_Play(uint32_t period){
	if(period==0){
		time = 0;
		DAC_Out(0);
	}
	NVIC_ST_CTRL_R	|= 0x10000; //flag = 1
	NVIC_ST_RELOAD_R = period;
//index of an array of notes
}


void SysTick_Handler(void){
	//output 1 or 0 to DAC_out
//	GPIO_PORTF_DATA_R ^=0x08; //heartbeat at PF3
	DAC_Out(wave[time]); 
	time = (time + 1)%32;
}
