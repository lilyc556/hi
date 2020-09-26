;****************** main.s ***************
; Program initially written by: Yerraballi and Valvano
; Author: Lillian Chieng
; Date Created: 1/15/2018 
; Last Modified: 2/3/2020
; Brief description of the program: Solution to Lab1
; The objective of this system is to implement odd-bit counting system
; Hardware connections: 
;  Output is positive logic, 1 turns on the LED, 0 turns off the LED
;  Inputs are negative logic, meaning switch not pressed is 1, pressed is 0
;    PE0 is an input 
;    PE1 is an input 
;    PE2 is an input 
;    PE3 is the output
; Overall goal: 
;   Make the output 1 if there is an odd number of switches pressed, 
;     otherwise make the output 0

; The specific operation of this system 
;   Initialize Port E to make PE0,PE1,PE2 inputs and PE3 an output
;   Over and over, read the inputs, calculate the result and set the output
; PE2  PE1 PE0  | PE3
; 0    0    0   |  1    odd number of 0’s
; 0    0    1   |  0    even number of 0’s
; 0    1    0   |  0    even number of 0’s
; 0    1    1   |  1    odd number of 0’s
; 1    0    0   |  0    even number of 0’s
; 1    0    1   |  1    odd number of 0’s
; 1    1    0   |  1    odd number of 0’s
; 1    1    1   |  0    even number of 0’s
;There are 8 valid output values for Port E: 0x01,0x02,0x04,0x07,0x08,0x0B,0x0D, and 0x0E. 

; NOTE: Do not use any conditional branches in your solution. 
;       We want you to think of the solution in terms of logical and shift operations

GPIO_PORTE_DATA_R  EQU 0x400243FC
GPIO_PORTE_DIR_R   EQU 0x40024400
GPIO_PORTE_DEN_R   EQU 0x4002451C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608

       THUMB		;16 bits
       AREA    DATA, ALIGN=2	;ram 
;global variables go here
      ALIGN
      AREA    |.text|, CODE, READONLY, ALIGN=2	;rom
      EXPORT  Start
Start
    LDR		R0,	=SYSCTL_RCGCGPIO_R ;turns on clock
	LDRB	R1,	[R0]
	ORR		R1,	#0x10
	STRB	R1,	[R0]
	
	NOP		;stabilize clock			
	NOP
	
	LDR		R0,	=GPIO_PORTE_DIR_R	;indicating output n input pins
	LDRB	R1,	[R0]
	AND		R1,	#0xF8
	ORR		R1,	#0x08
	STRB	R1,	[R0]
	
	LDR		R0,	=GPIO_PORTE_DEN_R
	LDRB	R1,	[R0]
	ORR		R1,	#0xF
	STRB	R1,	[R0]
	
	;code to run once that initializes PE3,PE2,PE1,PE0
loop
	LDR		R0,	=GPIO_PORTE_DATA_R
	LDR 	R10, [R0]
	AND		R1,	R10, #0x01 ; R1 = PE0
	AND		R2,	R10,	#0x02 ; R2 = PE1
	LSR		R2,	#1
	AND		R3, R10, #0x04 ; R3 = PE2
	LSR		R3,	#2
	EOR		R4, R1, R2
	EOR		R4,	R4,	R3
	EOR		R4, R4,	#1	 ; R4 = LED ON/OFF (toggled)
	
	;LSL		R10,	R10,	#3	 ; store R4 into PE3
	;BIC		R10,	R10,	#0x01
	;ADD		R10,	R10,	R4
	;LSR		R10,	R10,	#3
	
	LSL		R4,	#3
	STR		R4,[R0]
    ;code that runs over and over
	B    loop


    
    ALIGN        ; make sure the end of this section is aligned
    END          ; end of file
          