// Lab6.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a 4-key digital piano
// MOOC lab 13 or EE319K lab6 starter
// Program written by: Lily Chieng and Ashiqa Momin
// Date Created: 3/6/17 
// Last Modified: 1/17/2020 
// Lab number: 6
// Hardware connections
// PE0-3 Piano key inputs, PB0-3 DAC Outputs, PF3 Heartbeat, PD3 Oscilloscope


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "TExaS.h"
#include "dac.h"

#define C0  4778   // 523.3 Hz			
#define D   4257   // 587.3 Hz
#define E    3792   // 659.3 Hz
#define G    3189   // 784 Hz
uint8_t pianokey = 0;	
uint32_t hbcount = 0;

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

void Key_Press(){
	if (pianokey == 0x01) {Sound_Play(C0);}
	if (pianokey == 0x02) {Sound_Play(D);}
	if (pianokey == 0x04) {Sound_Play(E);}
	if (pianokey == 0x08) {Sound_Play(G);}
	if (pianokey == 0x00) {
		NVIC_ST_CTRL_R	&= ~0x10000;
		Sound_Play(0);
	} //flag clear
}

int main(void){      
  TExaS_Init(SW_PIN_PE3210,DAC_PIN_PB3210,ScopeOn);    // bus clock at 80 MHz & PD3 oscilloscope
  Piano_Init();
  Sound_Init();
  EnableInterrupts();

	SYSCTL_RCGCGPIO_R |= 0x20; // PF
	uint32_t volatile delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTF_DIR_R	|= 0x08; // PF3 heartbeat
	GPIO_PORTF_DEN_R	|= 0x08;		
	
  while(1){ 
		
		if(hbcount ==100000){
			GPIO_PORTF_DATA_R ^=0x08; //PF3 hearbeat toggle
			hbcount =0;
		}
		else{hbcount++;}
		
		//read piano key
		//call sound_play and quiet flag if no key pressed
		pianokey = Piano_In();
		Key_Press();
  }    
}



//TESTING DAC
/*
#include "DAC.h"
int main(void){      uint32_t data=0; // 0 to 15 DAC output
  TExaS_Init(SW_PIN_PE3210,DAC_PIN_PB3210,ScopeOn);    // bus clock at 80 MHz & PD3 oscilloscope
	DAC_Init();	
	
  while(1) {
    DAC_Out(data);
    data = 0x0F&(data+1); // 0,1,2...,14,15,0,1,2,...
  }    
}
*/



