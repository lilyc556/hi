// Piano.c
// This software configures the off-board piano keys
// Lab 6 requires a minimum of 4 keys, but you could have more
// Runs on LM4F120 or TM4C123
// Program written by: Lily Chieng and Ashiqa Momin
// Date Created: 3/6/17 
// Last Modified: 4/4/2020 
// Lab number: 6
// Hardware connections
// PE0-3 Piano key inputs, PB0-3 DAC Outputs, PF3 Heartbeat, PD3 Oscilloscope

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// **************Piano_Init*********************
// Initialize four piano key inputs, called once to initialize the digital ports
// Input: none 
// Output: none
void Piano_Init(void){ 
	//init piano key inputs
	SYSCTL_RCGCGPIO_R |= 0x10; //PE
	uint32_t volatile delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_PDR_R	|= 0x0F;
	GPIO_PORTE_DIR_R	&= ~0x0F; //PE0-3 inputs
	GPIO_PORTE_DEN_R	|= 0x0F; 
}
// **************Piano_In*********************
// Input from piano key inputs 
// Input: none 
// Output: 0 to 15 depending on keys
//   0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2, 0x08 is just Key3
//   bit n is set if key n is pressed
uint8_t key = 0;
uint32_t Piano_In(void){
	//read and mask input
	key = GPIO_PORTE_DATA_R & 0x0F;
  return key; // 
}
