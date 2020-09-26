// StepperMotorController.c starter file EE319K Lab 5
// Runs on TM4C123
// Finite state machine to operate a stepper motor.  
// Jonathan Valvano
// 3/8/2020
//Lily Chieng and Ashiqa Momin

// Hardware connections (External: two input buttons and four outputs to stepper motor)
//  PA5 is Wash input  (1 means pressed, 0 means not pressed)
//  PA4 is Wiper input  (1 means pressed, 0 means not pressed)
//  PE5 is Water pump output (toggle means washing)
//  PE4-0 are stepper motor outputs 
//  *PF1* PF2 or PF3 control the LED on Launchpad used as a heartbeat

#include "SysTick.h"
#include "TExaS.h"
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

void EnableInterrupts(void);
// edit the following only if you need to move pins from PA4, PE3-0      
// logic analyzer on the real board
#define PA4       (*((volatile unsigned long *)0x40004040))
#define PE50      (*((volatile unsigned long *)0x400240FC))
	
#define Home				0
//wipe forward
#define	WIPFWD2			1
#define	WIPFWD4			2
#define	WIPFWD8			3
#define	WIPFWD16		4
#define xWIPFWD1		5
#define xWIPFWD2		6
#define xWIPFWD4		7
#define xWIPFWD8		8
#define xWIPFWD16		9
//wipe backwards
#define WIPBKWD8		10
#define WIPBKWD4		11
#define WIPBKWD2		12
#define WIPBKWD1		13
#define xWIPBKWD16	14
#define xWIPBKWD8		15
#define xWIPBKWD4		16
#define xWIPBKWD2		17
//wash forward
#define	WASHFWD2		18
#define	WASHFWD4		19
#define	WASHFWD8		20
#define	WASHFWD16		21
#define xWASHFWD1		22
#define xWASHFWD2		23
#define xWASHFWD4		24
#define xWASHFWD8		25
#define xWASHFWD16	26
//wash backwards
#define WASHBKWD8		27
#define WASHBKWD4		28
#define WASHBKWD2		29
#define WASHBKWD1		30
#define xWASHBKWD16	31
#define xWASHBKWD8	32
#define xWASHBKWD4	33
#define xWASHBKWD2	34

	struct	State{
		uint8_t NextState[4];
		uint8_t output;
		uint8_t wait;
		uint8_t washLED;
	};
	typedef struct State wiperstate_t;
	
	const wiperstate_t wiperFSM[35]= {  //will need a total of 35
	//17 WIPER STATES + HOME 
	{ {Home,					WIPFWD2,		WASHFWD2,		WASHFWD2		},		1,	50,		0},
	{ { WIPFWD4,   		WIPFWD4, 		WASHFWD4,		WASHFWD4		},		2,	50,		0},
	{ { WIPFWD8,   		WIPFWD8, 		WASHFWD8,		WASHFWD8		},		4,	50,		0},
	{ {	WIPFWD16,			WIPFWD16,		WASHFWD16, 	WASHFWD16		},		8,	50,		0},
	{ {	xWIPFWD1,			xWIPFWD1,		xWASHFWD1, 	xWASHFWD1		},		16,	50,		0},
	{ {	xWIPFWD2,			xWIPFWD2,		xWASHFWD2, 	xWASHFWD2		},		1,	50,		0},
	{ {	xWIPFWD4,			xWIPFWD4,		xWASHFWD4, 	xWASHFWD4		},		2,	50,		0},
	{ {	xWIPFWD8,			xWIPFWD8,		xWASHFWD8, 	xWASHFWD8		},		4,	50,		0},
	{ {	xWIPFWD16,		xWIPFWD16,	xWASHFWD16, xWASHFWD16	},		8,	50,		0},
	{ {	WIPBKWD8,			WIPBKWD8,		WASHBKWD8, 	WASHBKWD8		},		16,	50,		0},
	{ {	WIPBKWD4,			WIPBKWD4,		WASHBKWD4, 	WASHBKWD4		},		8,	50,		0},
	{ {	WIPBKWD2,			WIPBKWD2,		WASHBKWD2, 	WASHBKWD2		},		4,	50,		0},
	{ {	WIPBKWD1,			WIPBKWD1,		WASHBKWD1, 	WASHBKWD1		},		2,	50,		0},
	{ {	xWIPBKWD16,		xWIPBKWD16,	xWASHBKWD16,xWASHBKWD16	},		1,	50,		0},
	{ {	xWIPBKWD8,		xWIPBKWD8,	xWASHBKWD8, xWASHBKWD8	},		16,	50,		0},
	{ {	xWIPBKWD4,		xWIPBKWD4,	xWASHBKWD4, xWASHBKWD4	},		8,	50,		0},
	{ {	xWIPBKWD2,		xWIPBKWD2,	xWASHBKWD2, xWASHBKWD2	},		4,	50,		0},
	{ {	Home,					Home,				Home, 			Home				},		2,	50,		0},
	// 17 WASHER STATES
	{ { WIPFWD4,   		WIPFWD4, 		WASHFWD4,		WASHFWD4		},		2,	50,		0x20},
	{ { WIPFWD8,   		WIPFWD8, 		WASHFWD8,		WASHFWD8		},		4,	50,		0},
	{ {	WIPFWD16,			WIPFWD16,		WASHFWD16, 	WASHFWD16		},		8,	50,		0x20},
	{ {	xWIPFWD1,			xWIPFWD1,		xWASHFWD1, 	xWASHFWD1		},		16,	50,		0},
	{ {	xWIPFWD2,			xWIPFWD2,		xWASHFWD2, 	xWASHFWD2		},		1,	50,		0x20},
	{ {	xWIPFWD4,			xWIPFWD4,		xWASHFWD4, 	xWASHFWD4		},		2,	50,		0},
	{ {	xWIPFWD8,			xWIPFWD8,		xWASHFWD8, 	xWASHFWD8		},		4,	50,		0x20},
	{ {	xWIPFWD16,		xWIPFWD16,	xWASHFWD16, xWASHFWD16	},		8,	50,		0},
	{ {	WIPBKWD8,			WIPBKWD8,		WASHBKWD8, 	WASHBKWD8		},		16,	50,		0x20},
	{ {	WIPBKWD4,			WIPBKWD4,		WASHBKWD4, 	WASHBKWD4		},		8,	50,		0},
	{ {	WIPBKWD2,			WIPBKWD2,		WASHBKWD2, 	WASHBKWD2		},		4,	50,		0x20},
	{ {	WIPBKWD1,			WIPBKWD1,		WASHBKWD1, 	WASHBKWD1		},		2,	50,		0},
	{ {	xWIPBKWD16,		xWIPBKWD16,	xWASHBKWD16,xWASHBKWD16	},		1,	50,		0x20},
	{ {	xWIPBKWD8,		xWIPBKWD8,	xWASHBKWD8, xWASHBKWD8	},		16,	50,		0},
	{ {	xWIPBKWD4,		xWIPBKWD4,	xWASHBKWD4, xWASHBKWD4	},		8,	50,		0x20},
	{ {	xWIPBKWD2,		xWIPBKWD2,	xWASHBKWD2, xWASHBKWD2	},		4,	50,		0},
	{ {	Home,					Home,				Home, 			Home				},		2,	50,		0x20}
};
	
void SendDataToLogicAnalyzer(void){
  UART0_DR_R = 0x80|(PA4<<2)|PE50;
}

void	Heartbeat(void){
	GPIO_PORTF_DATA_R ^= 0x02;
}

int main(void){ 
  TExaS_Init(&SendDataToLogicAnalyzer);    // activate logic analyzer and set system clock to 80 MHz
  SysTick_Init();   
	// you initialize your system here
	SYSCTL_RCGCGPIO_R |= 	0x31; //Clock ON for PA, PE, PF
	GPIO_PORTA_DIR_R	&= ~0x30; //PA4,PA5 wiper & washer inputs
	GPIO_PORTA_DEN_R	|=	0x30; //PA4,PA5 
	GPIO_PORTF_LOCK_R	 =	0x4C4F434B; //unlock PF
	GPIO_PORTF_CR_R		|=	0x01;	//allow PF1 changes
	GPIO_PORTF_DIR_R	|=	0x02;	//PF1 control LED on launchpad (heartbeat)
	GPIO_PORTF_DEN_R	|= 	0x02; //PF1
	//GPIO_PORTF_PDR_R	|=	0x01;	//PF1 pull down resistor but not a switch
	GPIO_PORTE_DIR_R	|=	0x3F;	//PE4-0 motor outputs, PE5 water pump output
	GPIO_PORTE_DEN_R	|=	0x3F;	//PE4-0, PE5

	//AMSEL 	disable analog function
	//PCTL		enable regular GPIO
	//AFSEL		enable digital function

  EnableInterrupts();   


	
	uint8_t current = Home;
	uint8_t input; //PA5,PA4

	
  while(1){
	uint32_t portEOutput = wiperFSM[current].washLED;
	portEOutput = portEOutput + wiperFSM[current].output;			// output
	GPIO_PORTE_DATA_R = portEOutput; 
	SysTick_Wait(wiperFSM[current].wait);						// wait 
	input = GPIO_PORTA_DATA_R &= 0x30;								// input
	input = input >>4;
	current = wiperFSM[current].NextState[input];		// next	
	//GPIO_PORTE_DATA_R &= ~0x20;
	//GPIO_PORTE_DATA_R |= wiperFSM[current].washLED;
	Heartbeat();
  }
	
}


