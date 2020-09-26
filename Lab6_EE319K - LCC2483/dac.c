// dac.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on LM4F120 or TM4C123
// Program written by: Lily Chieng and Ashiqa Momin
// Date Created: 3/6/17 
// Last Modified: 1/17/2020 
// Lab number: 6
// Hardware connections
// PE0-3 Piano key inputs, PB0-3 DAC Outputs, PF3 Heartbeat, PD3 Oscilloscope

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	//init port b as output
	SYSCTL_RCGCGPIO_R |= 0x02; // PB
	uint32_t volatile delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTB_DIR_R	|= 0x0F; // PB0-3 DAC output
	GPIO_PORTB_DEN_R	|= 0x0F;		
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data){
	//write a 4 bit value to port B
	GPIO_PORTB_DATA_R = data;
}
