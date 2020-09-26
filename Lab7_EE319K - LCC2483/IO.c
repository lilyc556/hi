// IO.c
// This software configures the switch and LED
// You are allowed to use any switch and any LED, 
// although the Lab suggests the SW1 switch PF4 and Red LED PF1
// Runs on LM4F120 or TM4C123
// Program written by: Lily Chieng, Ashiqa Momin
// Date Created: March 30, 2018
// Last Modified:  4/12/2020
// Lab number: 7


#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>

//------------IO_Init------------
// Initialize GPIO Port for a switch and an LED
// Input: none
// Output: none
void IO_Init(void) {
	SYSCTL_RCGCGPIO_R |= 0x30; //PORT F + E
	uint32_t volatile delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTF_LOCK_R	= 0x4C4F434B;
	GPIO_PORTF_CR_R	= 0x1F;
	GPIO_PORTF_DIR_R |= 0x04; //PF2 output heartbeat led 
	GPIO_PORTF_DIR_R &= ~0x10; //PF4 input switch
	GPIO_PORTF_PUR_R	|= 0x10;
	//GPIO_PORTF_AFSEL_R &= ~0x10;
	GPIO_PORTF_DEN_R |= 0x14;
		
	
 // --UUU-- Code to initialize PF4 and PF2
}

//------------IO_HeartBeat------------
// Toggle the output state of the  LED.
// Input: none
// Output: none
void IO_HeartBeat(void) {
 // --UUU-- PF2 is heartbeat
	GPIO_PORTF_DATA_R ^= 0x04;
}


//------------IO_Touch------------
// wait for release and press of the switch
// Delay to debounce the switch
// Input: none
// Output: none
void IO_Touch(void) {
 // --UUU-- wait for release; delay for 20ms; and then wait for press
	while(!(GPIO_PORTF_DATA_R & 0x10));
	uint32_t delay = 400000; // 20 ms 
	while(delay!=0){ delay--;}
	while((GPIO_PORTF_DATA_R & 0x10));
	delay = 400000;
	while(delay!=0){ delay--;}
	
}  

