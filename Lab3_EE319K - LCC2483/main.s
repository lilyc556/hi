;****************** main.s ***************
; Program written by: Lily Chieng, Ashiqa Momin
; Date Created: 2/4/2017
; Last Modified: 2/11/2020
; Brief description of the program
;   The LED toggles at 2 Hz and a varying duty-cycle
; Hardware connections (External: One button and one LED)
;  PE1 is Button input  (1 means pressed, 0 means not pressed)
;  PE2 is LED output (1 activates external LED on protoboard)
;  PF4 is builtin button SW1 on Launchpad (Internal) 
;        Negative Logic (0 means pressed, 1 means not pressed)
; Overall functionality of this system is to operate like this
;   1) Make PE2 an output and make PE1 and PF4 inputs.
;   2) The system starts with the the LED toggling at 2Hz,
;      which is 2 times per second with a duty-cycle of 30%.
;      Therefore, the LED is ON for 150ms and off for 350 ms.
;   3) When the button (PE1) is pressed-and-released increase
;      the duty cycle by 20% (modulo 100%). Therefore for each
;      press-and-release the duty cycle changes from 30% to 70% to 70%
;      to 90% to 10% to 30% so on
;   4) Implement a "breathing LED" when SW1 (PF4) on the Launchpad is pressed:
;      a) Be creative and play around with what "breathing" means.
;         An example of "breathing" is most computers power LED in sleep mode
;         (e.g., https://www.youtube.com/watch?v=ZT6siXyIjvQ).
;      b) When (PF4) is released while in breathing mode, resume blinking at 2Hz.
;         The duty cycle can either match the most recent duty-
;         cycle or reset to 30%.
;      TIP: debugging the breathing LED algorithm using the real board.
; PortE device registers
GPIO_PORTE_DATA_R  EQU 0x400243FC
GPIO_PORTE_DIR_R   EQU 0x40024400
GPIO_PORTE_AFSEL_R EQU 0x40024420
GPIO_PORTE_DEN_R   EQU 0x4002451C
; PortF device registers
GPIO_PORTF_DATA_R  EQU 0x400253FC
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_PUR_R   EQU 0x40025510
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_LOCK_R  EQU 0x40025520
GPIO_PORTF_CR_R    EQU 0x40025524
GPIO_LOCK_KEY      EQU 0x4C4F434B  ; Unlocks the GPIO_CR register
SYSCTL_RCGCGPIO_R  EQU 0x400FE608
count1			   EQU 3000000
count2			   EQU 7000000

       IMPORT  TExaS_Init
       THUMB
       AREA    DATA, ALIGN=2
;global variables go here

       AREA    |.text|, CODE, READONLY, ALIGN=2
;DutyON			   DCD 3000000,5000000,7000000,9000000,1000000	;2Hz
;DutyOFF		   DCD 7000000,5000000,3000000,1000000,9000000	;2Hz
	
DutyON			   DCD 6000,8000,10000,12000,14000,16000,18000,20000,0,2000,4000;1000Hz 
DutyOFF			   DCD 14000,12000,10000,8000,6000,4000,2000,0,20000,18000,16000 ;1000Hz
       THUMB
       EXPORT  Start
Start
 ; TExaS_Init sets bus clock at 80 MHz
     BL  TExaS_Init ; voltmeter, scope on PD3
	 
 ; Initialization goes here
			LDR		R0,	=SYSCTL_RCGCGPIO_R 
			LDRB	R1,	[R0]
			ORR		R1,	#0x30 ;clock on for PORTE&F
			STRB	R1,	[R0]
			
			NOP				
			NOP
			
			LDR		R0,	=GPIO_PORTE_DIR_R	;0 input, 1 output
			LDRB	R1,	[R0]
			AND		R1,	#0xFD	;PE1 input
			ORR		R1,	#0x04	;PE2 output
			STRB	R1,	[R0]
			
			LDR		R0,	=GPIO_PORTE_DEN_R
			LDRB	R1,	[R0]
			ORR		R1,	#0x06	;turn on PE1,2
			STRB	R1,	[R0]
		
			LDR		R0,	=GPIO_PORTF_LOCK_R	;Unlock port F
			LDR		R2,	=0x4C4F434B
			STRB	R2,	[R0]
			
			LDR		R0,	=GPIO_PORTF_CR_R	;allows changes to poRT F
			LDR		R1,[R0]
			ORR		R1,	#0x1F
			STRB	R1,	[R0]
			
			LDR		R0,	=GPIO_PORTF_DEN_R
			LDRB	R1,	[R0]
			ORR		R1,	#0x10
			STRB	R1,	[R0]
			
			LDR		R0,=GPIO_PORTF_DIR_R
			LDRB	R1,[R0]
			AND		R1,	#0xEF	;PF4 input
			STRB	R1,[R0]
			
			LDR		R0,	=GPIO_PORTF_PUR_R	;active pull up on the PF4
			LDRB	R1,[R0]
			ORR		R1,#0x10
			STRB	R1,[R0]
			
		
			MOV		R3, #0x00		;R3 = array counter/offset
			LDR		R4,	=DutyON		;R4	= DutyON  array
			LDR		R5,	=DutyOFF	;R5	= DutyOFF array
			
     CPSIE  I    ; TExaS voltmeter, scope runs on interrupts
loop  
; main engine goes here	
            BL 		ledon
			BL		keypress
			LDR		R0,	[R4,R3]	;R0 = 5,000,000
			BL		delay
			BL		stop
			BL 		ledoff
			BL		keypress
			LDR		R0,	[R5,R3]
			BL		delay	
			BL		stop
			B    	loop

delay		
			SUBS	R0,R0,#1
			BNE		delay
			BX		LR
ledon
			LDR		R0,	=GPIO_PORTE_DATA_R
			MOV     R2,	#0x04
			STRB	R2,	[R0]
			BX 		LR

ledoff
 			LDR		R0,	=GPIO_PORTE_DATA_R
			MOV     R2,	#0x00
			STRB	R2,	[R0]
			BX 		LR 
			
keypress	LDR		R0,	=GPIO_PORTF_DATA_R
			LDR		R1,	[R0]
			AND		R1,	#0x10	;PF4 = #16
			MOV		R2,	#0x10
			CMP		R1,R2
			BNE		skip	
			MOV		R8,	#0x28	
			CMP		R3,	R8 ;if R3 = 36 then go back to the front
			BEQ		resetDuty	
			ADD		R3,	#4
			BX		LR
			
resetDuty	MOV		R3,	#0x00
			B		skip

stop		LDR		R0,	=GPIO_PORTE_DATA_R
			LDR		R6,	[R0]
			AND		R6,	R6,	#0x2	;R6 = PE1
			MOV		R7,	#0x02		;R7	=	2
			CMP		R6, R7
			BNE		skip
			
pauseloop	LDR		R0,	=GPIO_PORTE_DATA_R
			LDR		R6,	[R0]
			AND		R6,	R6,	#0x2
			CMP		R6,R7
			BEQ		pauseloop
skip		BX		LR
			
			ALIGN      ; make sure the end of this section is aligned
			END        ; end of file

