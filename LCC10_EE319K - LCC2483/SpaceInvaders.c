// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/17/2020 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2019

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2019

 Copyright 2019 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include <stdlib.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"

typedef enum {english,spanish} lang_t;
lang_t language; 
uint8_t choseLang;
uint32_t newPosition;
uint32_t oldPosition = 0;
uint16_t score = 0;
int8_t lives = 3;
uint32_t lastShoot;
uint32_t nowShoot;
uint32_t screenLasers, anyLasers, laserIndex = 0;
uint8_t  pauseGame = 0;
uint8_t  moveY = 0;
	
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

//Start Screen
void startScreen(void){
	ST7735_FillScreen(0x0000);
	//ST7735_SetCursor(1, 1)
  //ST7735_OutString("SPACE INVADERS");
	ST7735_DrawBitmap(1, 47, spaceinv, 110, 46);
	ST7735_SetCursor(1, 7);
  ST7735_OutString("ENGLISH");
	ST7735_SetCursor(1, 9);
  ST7735_OutString("ESPA\xA5OL");
	ST7735_SetCursor(1, 14);
  ST7735_OutString("GREEN = english");
	ST7735_SetCursor(1, 15);
	ST7735_OutString("RED = espa\xA4ol");
	
	while(choseLang !=1){
		if(GPIO_PORTE_DATA_R & 0x02){ //RED = SPANISH
			ST7735_DrawBitmap(50, 97, arrow, 8, 9);
			language = spanish;
			Sound_Menu();
			choseLang = 1;
		}
		
		if(GPIO_PORTE_DATA_R & 0x01){ //GREEN = ENGLISH
			ST7735_DrawBitmap(50, 77, arrow, 8, 9);
			language = english;
			Sound_Menu();
			choseLang = 1;
		}
	}
}


//BUTTON INITIALIZATIONS (PE1-0)
void Button_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x10;
	GPIO_PORTE_DIR_R &= ~0x03;
	GPIO_PORTE_DEN_R |= 0x03;
}

//SPRITE STRUCT
typedef enum {dead,alive} status_t;
struct sprite{
	int32_t x;
	int32_t y;
	int32_t vx,vy; // velocity pixels 30 Hz
	const unsigned short *image; //ptr to image
	const unsigned short *black; //dead/alive
	status_t life;
	uint32_t width;
	uint32_t height;
	uint32_t needDraw;
};

typedef struct sprite sprite_t;

//GAME INITIALIZATION
sprite_t enemy[18];
int anyAlive;

void GameInit(void){
	//enemyFlag = 0;
	anyAlive = 1;
	for(int i =0; i<6; i++){
		enemy[i].x = 20*i;
		enemy[i].y = 30;
		enemy[i].vx = 1; 
		enemy[i].vy = 1;
		enemy[i].image = SmallEnemy20pointA;
		enemy[i].black = BlackEnemy; 
		enemy[i].life = alive;
		enemy[i].width = 16;
		enemy[i].height = 10;
		enemy[i].needDraw = 1;
	}
	
	int j = 0;
	for(int i = 6; i<11; i++){
		enemy[i].x = (20*j) + 10;
		enemy[i].y = 43;
		enemy[i].vx = 1; 
		enemy[i].vy = 1;
		enemy[i].image = SmallEnemy10pointA;
		enemy[i].black = BlackEnemy; 
		enemy[i].life = alive;
		enemy[i].width = 16;
		enemy[i].height = 10;
		enemy[i].needDraw = 1;
		j++;
	}
		int k = 0;
		for(int i = 11; i<17; i++){
		enemy[i].x = (20*k);
		enemy[i].y = 56;
		enemy[i].vx = 1; 
		enemy[i].vy = 1;
		enemy[i].image = SmallEnemy20pointB;
		enemy[i].black = BlackEnemy; 
		enemy[i].life = alive;
		enemy[i].width = 16;
		enemy[i].height = 10;
		enemy[i].needDraw = 1;
		k++;
	}
	
}

//ENEMY MOVE 30Hz Interrupt
void GameMove(void){ 
	anyAlive = 0;
	moveY = (moveY+1)%25;
			if(enemy[0].x == 0){
				for(int z = 0; z<18; z++){
				enemy[z].vx = 1;
				}
			}

			if(enemy[5].x == 114){
				for(int z = 0; z<18; z++){
				enemy[z].vx = -1;
				}
			}
			
			for(int i =0; i<18; i++){
				
				if(enemy[i].life == alive){
					enemy[i].needDraw = 1;
					anyAlive = 1;
					
					if(enemy[i].x <0){enemy[i].life = dead;}
					if(enemy[i].x >128){enemy[i].life = dead;}
					if(enemy[i].y <30){enemy[i].life = dead;}
					if(enemy[i].y >152) {enemy[i].life = dead; lives--;}
					
					enemy[i].x += enemy[i].vx;
					if(moveY == 24){enemy[i].y += enemy[i].vy;}
				//	enemy[i].y += enemy[i].vy;
				}
			}
			
				if(enemy[0].life == dead){enemy[0].x += enemy[0].vx;}
				if(enemy[5].life == dead){enemy[5].x += enemy[5].vx;}

}

//MOVE ENEMY ISR
uint32_t enemyFlag = 0; //semaphore

void moveEnemy (void){
		enemyFlag = 1;
	//else{NVIC_DIS0_R = 1<<21;} // disable bit 21 in IQR 
}

void Moving_Init(void){ 
	Timer1_Init(&moveEnemy, (2666666));	//30 Hz
}

//CREATE LASER 30Hz Interrupt
sprite_t laser[100]; //prob only 100 lasers on screen
void addLaser(void){
		screenLasers++;
		laser[laserIndex].x = oldPosition + 7;
		laser[laserIndex].y = 157;
		laser[laserIndex].vx = 0; //not moving initially
		laser[laserIndex].vy = -1;
		laser[laserIndex].image = laserimg;
		laser[laserIndex].black = blackLaser; 
		laser[laserIndex].life = alive;
		laser[laserIndex].width = 4;
		laser[laserIndex].height = 6;
		laser[laserIndex].needDraw = 1;		
		laserIndex = (laserIndex + 1)%100; //array of lasers
	
	
	if(screenLasers != 0){anyLasers = 1;}
	else{anyLasers = 0;}	
}

//MOVE LASER 30Hz Interrupt
void moveLaser(void){
	if(anyLasers == 1){
		for(int i = 0; i<screenLasers; i++){
			if(laser[i].y < 30){laser[i].life = dead;}
			if(laser[i].life == alive){
				
				laser[i].y--;
			}
		//	if(laser[i].life == dead){laser[i].x = 200;}//offscreen
		}
	}
}

//ENEMY DRAW 
void GameDraw(void){
	for(int i = 0; i<18; i++){		//draw enemies
		if(enemy[i].needDraw){
			if(enemy[i].life == alive){
				ST7735_DrawBitmap(enemy[i].x, enemy[i].y, enemy[i].image, enemy[i].width, enemy[i].height);
			}
			else{
				ST7735_DrawBitmap(enemy[i].x, enemy[i].y, enemy[i].black, enemy[i].width, enemy[i].height);
			//	enemy[i].x = 300; //offscreen
			}
		//	enemy[i].needDraw = 0;
		}
	}
	
	for(int i = 0; i<100; i++){		//draw lasers
		if(laser[i].needDraw){
			if(laser[i].life == alive){
				ST7735_DrawBitmap(laser[i].x, laser[i].y, laser[i].image, laser[i].width, laser[i].height);
			}
			else{
				ST7735_DrawBitmap(laser[i].x, laser[i].y, laser[i].black, laser[i].width, laser[i].height);
			//	laser[i].x = 200;//offscreen
			}
			//else{}
		}
	}
}

//SYSTICK POTENTIOMETER ISR
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = 2666666; //30Hz 
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 0x07;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000; // priority of 1?
}

uint32_t ADCMail;
uint32_t ADCStatus;

void SysTick_Handler(void){
	ADCMail = ADC_In(); //sample ADC
	ADCStatus = 1; //new data 
}

//ADC CONVERT
uint32_t Convert(uint32_t input){
		return (109*input/4096 + (13/10));
}

//PLAYER POSITION
void PlayerPosition(void){
		// && ((newPosition>2) | (newPosition <158)) put it after calibrated right
		while((newPosition > oldPosition)){
			oldPosition+=2;
			ST7735_DrawBitmap(oldPosition, 159, PlayerShip0, 18,8); 
		}
		while(newPosition < oldPosition){
			oldPosition-=2;
			ST7735_DrawBitmap(oldPosition, 159, PlayerShip0, 18,8); 
		}
}

//SCORE/LIVES UPDATE
void scoreUpdate(void){
	if(language == english){
		ST7735_SetCursor(1, 1);
		ST7735_OutString("Score: ");
		LCD_OutDec(score);
		ST7735_SetCursor(12, 1);
		ST7735_OutString("Lives: ");
		LCD_OutDec(lives);
	}
	if(language== spanish){
	ST7735_SetCursor(1, 1);
	ST7735_OutString("Tanto: ");
	LCD_OutDec(score);
	ST7735_SetCursor(12, 1);
	ST7735_OutString("Vive: ");
	LCD_OutDec(lives);
	}
}

//PLAY SOUNDS
/*
void game_sound(void){
	if((GPIO_PORTE_DATA_R & 0x01) == 0x01){
		Sound_Shoot();
		addLaser();
	}
}
*/
void game_sound(void){
	while((GPIO_PORTE_DATA_R & 0x01) == 0x01){
		lastShoot = 1;
	}
	if(!(GPIO_PORTE_DATA_R & 0x01)){
		if(lastShoot == 1){
			lastShoot = 0;
			Sound_Shoot();
			addLaser();
		}
		
	}
}

//HIT ENEMY
int8_t xtolerance = 16; //width of sprite
int8_t ytolerance = 10;
void hitEnemy(void){
	
	for(int i = 0; i<18; i++){		
		if(enemy[i].life == alive){
				for(int j = 0; j<100; j++){		
					if(laser[j].life == alive){
						if(((enemy[i].y - laser[j].y) <= ytolerance) && ((enemy[i].y - laser[j].y) > 0) && ((laser[j].x - enemy[i].x) <= xtolerance) && ((laser[j].x - enemy[i].x) > 0)){
						//if(((laser[j].y - 6) == enemy[i].y) && ((laser[j].x - enemy[i].x) <= xtolerance) && ((laser[j].x - enemy[i].x) > 0)){
							laser[j].life = dead;
							enemy[i].life = dead;
							if(enemy[i].image == SmallEnemy20pointA){score += 20;};
							if(enemy[i].image == SmallEnemy20pointB) {score +=20;};
							if(enemy[i].image == SmallEnemy10pointA){score += 10;};
							break;
						}								
					}
				}
		}
	}
	/*
	for(int j = 0; j<100; j++){
		for(int i = 0; i<18; i++){
			if(laser[j].life == alive){
				if(enemy[i].life == alive){
					if(((laser[j].y - 6) == enemy[i].y) && ((laser[j].x - enemy[i].x) <= xtolerance) && ((laser[j].x - enemy[i].x) > 0)){
						laser[j].life = dead;
						enemy[i].life = dead;
						if(enemy[i].image == SmallEnemy20pointA){score += 20;};
						if(enemy[i].image == SmallEnemy20pointB) {score +=20;};
						if(enemy[i].image == SmallEnemy10pointA){score += 10;};
						//break;
					}						
				}
			}
		}
	}
	*/
}
//CHECK PAUSE
void checkPause(void){
			if(GPIO_PORTE_DATA_R & 0x02){
			Sound_Menu();
			pauseGame = 1;
			}
}
//END GAME DISPLAYS
void loser(void){
	ST7735_FillScreen(0x0000);   // set screen to black
	Game_Over();
	Delay100ms(10);
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Human!");
  ST7735_SetCursor(1, 4);
	ST7735_OutString("Your Score: ");
  LCD_OutDec(score);
	exit(0);
}

void winner(void){
	ST7735_FillScreen(0x0000);   // set screen to black
	Vic_Tory();
	Delay100ms(10);
  ST7735_SetCursor(1, 1);
  ST7735_OutString("CONGRATULATIONS");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("ON SAVING THE WORLD");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("WAHOO!");
  ST7735_SetCursor(1, 4);
	ST7735_OutString("Your Score: ");
  LCD_OutDec(score);
}

void sploser(void){
	ST7735_FillScreen(0x0000);   // set screen to black
	Game_Over();
	Delay100ms(10);
  ST7735_SetCursor(1, 1);
  ST7735_OutString("FIN DEL JUEG0");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Buen intento,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Humano!");
  ST7735_SetCursor(1, 4);
  ST7735_OutString("Su Puntuaci\xA2n: ");
  LCD_OutDec(score);
	exit(0);
}

void spwinner(void){
  ST7735_FillScreen(0x0000);   // set screen to black
  Vic_Tory();
	Delay100ms(10);
  ST7735_SetCursor(1, 1);
  ST7735_OutString("FELICIDADES");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("POR SALVAR EL MUNDO");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("\xADHURRA!");
  ST7735_SetCursor(1, 4);
	ST7735_OutString("Su Puntuaci\xA2n: ");
  LCD_OutDec(score);
}

//LIVES COUNT
void livescount(void){
	if(lives <= 0){
			if(language == english){loser();}
			if(language == spanish){sploser();}
	} 
}

//MAIN METHOD
int main(void){
  DisableInterrupts();
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
 // Random_Init(1);
	
	ADC_Init();
	Button_Init();
	EnableInterrupts();
	SysTick_Init();
	Sound_Init();
	Moving_Init();
	GameInit();

  Output_Init();
	startScreen();
	
	Delay100ms(10);
  ST7735_FillScreen(0x0000);            // set screen to black
	GameDraw();														
	
	
	
//	ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
	/*
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
	
  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);
	*/
	
	//Delay100ms(50);              // delay 5 sec at 80 MHz

  while(anyAlive){
		scoreUpdate();
		livescount();
		
		//30Hz Interrupt
		if(enemyFlag == 1){
			enemyFlag = 0;
			game_sound();
			GameMove();
			GameDraw();
			moveLaser();
			hitEnemy();
			checkPause();

		}
		//player ship potentiometer
		if(ADCStatus == 1){
		newPosition = Convert(ADCMail);
		ADCStatus = 0;
		PlayerPosition();
		}
		
		//check pause at 30Hz
		if(pauseGame){
			while(!(GPIO_PORTE_DATA_R & 0x01));
			pauseGame = 0;
		}
  }
	
	
	if(language == english){winner();}
	if(language == spanish){spwinner();}
	//END GAME

}



//DELAY
// You can't use this ti                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              mer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
