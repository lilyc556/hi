;****************** main.s ***************
; Program written by: Lily Chieng, Ashiqa Momin
; Date Created: 2/14/2017
; Last Modified: 3/1/2020
; You are given a simple stepper motor software system with one input and
; four outputs. This program runs, but you are asked to add minimally intrusive
; debugging instruments to verify it is running properly. 
; The system outputs in one of three modes:
; 1) cycles through 10,6,5,9,... at a constant rate
; 2) cycles through 5,6,10,9,... at a constant rate
; 3) does not change the output, but still outputs at a constant rate
; When PA4 goes high and low again, the system cycles through these modes
; The output rate will be different on the simulator versus the real board
;   Insert debugging instruments which gather data (state and timing)
;   to verify that the system is functioning as expected.
; Hardware connections (External: One button and four outputs to stepper motor)
;  PA4 is Button input  (1 means pressed, 0 means not pressed)
;  PE3-0 are stepper motor outputs 
;  PF2 is Blue LED on Launchpad used as a heartbeat
; Instrumentation data to be gathered is as follows:
; After every output to Port E, collect one state and time entry. 
; The state information is the 5 bits on Port A bit 4 and Port E PE3-0
;   place one 8-bit entry in your Data Buffer  
; The time information is the 24-bit time difference between this output and the previous (in 12.5ns units)
;   place one 32-bit entry in the Time Buffer
;    24-bit value of the SysTick's Current register (NVIC_ST_CURRENT_R)
;    you must handle the roll over as Current goes 3,2,1,0,0x00FFFFFF,0xFFFFFE,
; Note: The size of both buffers is 100 entries. Once you fill these
;       entries you should stop collecting data
; The heartbeat is an indicator of the running of the program. 
; On each iteration of the main loop of your program toggle the 
; LED to indicate that your code(system) is live (not stuck or dead).

SYSCTL_RCGCGPIO_R  EQU 0x400FE608
NVIC_ST_CURRENT_R  EQU 0xE000E018
GPIO_PORTA_DATA_R  EQU 0x400043FC
GPIO_PORTA_DIR_R   EQU 0x40004400
GPIO_PORTA_DEN_R   EQU 0x4000451C
GPIO_PORTE_DATA_R  EQU 0x400243FC
GPIO_PORTE_DIR_R   EQU 0x40024400
GPIO_PORTE_DEN_R   EQU 0x4002451C
GPIO_PORTF_DATA_R  EQU 0x400253FC
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_DEN_R   EQU 0x4002551C
; RAM Area
		  PRESERVE8
          AREA    DATA, ALIGN=2
Index     SPACE 4 ; index into Stepper table 0,1,2,3
Direction SPACE 4 ; -1 for CCW, 0 for stop 1 for CW
	

;place your debug variables in RAM here
DataBuffer	SPACE	100
TimeBuffer	SPACE	400
DataPt		SPACE	4
TimePt		SPACE	4
Last		SPACE	4
; ROM Area
        IMPORT TExaS_Init
        IMPORT SysTick_Init
;-UUU-Import routine(s) from other assembly files (like SysTick.s) here
        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
Stepper DCB 5,6,10,9
        EXPORT  Start

Start
 ; TExaS_Init sets bus clock at 80 MHz
; PA4, PE3-PE0 out logic analyzer to TExasDisplay
      LDR  R0,=SendDataToLogicAnalyzer
      ORR  R0,R0,#1
      BL   TExaS_Init ; logic analyzer, 80 MHz
 ;place your initializations here
	  LDR	R0,=SYSCTL_RCGCGPIO_R
	  LDR 	R1,[R0]
	  ORR	R1,R1,#0x20
	  STR	R1,[R0]
	  NOP
	  NOP
	  
;	  LDR	R0,=GPIO_PORTF_LOCK_R
;	  LDR	R1,=0x4C4F434B
;	  STR	R1,[R0]
	  
;	  LDR	R0,=GPIO_PORTF_CR_R
;	  LDR	R1,=0x04
;	  STR	R1,[R0]
	  
	  LDR	R0,=GPIO_PORTF_DIR_R
	  LDR	R1,[R0]
	  ORR	R1,R1,#0x04
	  STR	R1,[R0]
	  
	  LDR	R0,=GPIO_PORTF_DEN_R
	  LDR	R1,[R0]
	  ORR	R1,R1,#0x04
	  STR	R1,[R0]
	  
      BL   Stepper_Init ; initialize stepper motor
      BL   Switch_Init  ; initialize switch input
;**********************
      BL   Debug_Init ;initialize debugger
;**********************
      CPSIE  I    ; TExaS logic analyzer runs on interrupts
      MOV  R5,#0  ; last PA4
loop  

      LDR  R1,=GPIO_PORTA_DATA_R
      LDR  R4,[R1]  ;current value of switch
      AND  R4,R4,#0x10 ; select just bit 4
      CMP  R4,#0
      BEQ  no     ; skip if not pushed
      CMP  R5,#0
      BNE  no     ; skip if pushed last time
      ; this time yes, last time no
      LDR  R1,=Direction
      LDR  R0,[R1]  ; current direction
      ADD  R0,R0,#1 ;-1,0,1 to 0,1,2
      CMP  R0,#2
      BNE  ok
      MOV  R0,#-1  ; cycles through values -1,0,1
ok    STR  R0,[R1] ; Direction=0 (CW)  
no    MOV  R5,R4   ; setup for next time
      BL   Stepper_Step               
      LDR  R0,=1600000
      BL   Wait  ; time delay fixed but not accurate   
      B    loop
;Initialize stepper motor interface
Stepper_Init
      MOV R0,#1
      LDR R1,=Direction
      STR R0,[R1] ; Direction=0 (CW)
      MOV R0,#0
      LDR R1,=Index
      STR R0,[R1] ; Index=0
    ; activate clock for Port E
      LDR R1, =SYSCTL_RCGCGPIO_R
      LDR R0, [R1]
      ORR R0, R0, #0x10  ; Clock for E
      STR R0, [R1]
      NOP
      NOP                 ; allow time to finish activating
    ; set direction register
      LDR R1, =GPIO_PORTE_DIR_R
      LDR R0, [R1]
      ORR R0, R0, #0x0F    ; Output on PE0-PE3
       STR R0, [R1]
    ; enable digital port
      LDR R1, =GPIO_PORTE_DEN_R
      LDR R0, [R1]
      ORR R0, R0, #0x0F    ; enable PE3-0
      STR R0, [R1]
      BX  LR
	  
;Initialize switch interface
Switch_Init
    ; activate clock for Port A
      LDR R1, =SYSCTL_RCGCGPIO_R
      LDR R0, [R1]
      ORR R0, R0, #0x01  ; Clock for A
      STR R0, [R1]
      NOP
      NOP                 ; allow time to finish activating
    ; set direction register
      LDR R1, =GPIO_PORTA_DIR_R
      LDR R0, [R1]
      BIC R0, R0, #0x10    ; Input on PA4
      STR R0, [R1]
    ; 5) enable digital port
      LDR R1, =GPIO_PORTA_DEN_R
      LDR R0, [R1]
      ORR R0, R0, #0x10    ; enable PA4
      STR R0, [R1]
      BX  LR
; Step the motor clockwise
; Direction determines the rotational direction
; Input: None
; Output: None
Stepper_Step
      PUSH {R4,LR}
      LDR  R1,=Index
      LDR  R2,[R1]     ; old Index
      LDR  R3,=Direction
      LDR  R0,[R3]     ; -1 for CCW, 0 for stop 1 for CW
      ADD  R2,R2,R0
      AND  R2,R2,#3    ; 0,1,2,3,0,1,2,...
      STR  R2,[R1]     ; new Index
      LDR  R3,=Stepper ; table
      LDRB R0,[R2,R3]  ; next output: 5,6,10,9,5,6,10,...
      LDR  R1,=GPIO_PORTE_DATA_R ; change PE3-PE0
      STR  R0,[R1]
      BL   Debug_Capture
      POP {R4,PC}
; inaccurate and inefficient time delay
Wait 
      SUBS R0,R0,#1  ; outer loop
      BNE  Wait 
      BX   LR
      
Debug_Init 
      PUSH {R0-R4,LR}
	  MOV	R2,	#0xFF
	  MOV	R3,	#100
	  LDR	R0,=DataBuffer
	  LDR	R1,	=DataPt
	  STR	R0,[R1]	
	  
dLoop STRB	R2,[R0]
	  ADD	R0,#1
	  SUBS	R3,#1
	  BNE	dLoop
	  
	  LDR	R2,	=0xFFFFFFFF
	  MOV	R3,	#100
	  LDR	R0,=TimeBuffer
	  LDR	R1,	=TimePt
	  STR	R0,[R1]
tLoop STR	R2,[R0]
	  ADD	R0,#4
	  SUBS	R3,#1
	  BNE	tLoop
	  
	  BL	SysTick_Init
      POP {R0-R4,PC}
      
Debug_Capture 
	  ;Estimate Intrusiveness
	  ;39 Instructions
	  ;12.5 ns * (39 instructions * 2cc) = 975 ns		Time to execute Debug_Capture instrument
	  ;6400149cc x 12.5ns = 80001862.5 ns				Time between Debug_Capture
	  ;975 / 80001862.5 = 0.0012%
	  PUSH {R0-R9,LR}
	  BL	Heart

	  LDR	R0,	=DataBuffer				;R0=DataBuffer Address
	  LDR	R9,	=TimeBuffer				;R9=TimeBuffer Address
	  LDRB	R1,	[R0,#99]
	  CMP	R1,	#0xFF
	  BNE	Full
	  ;MOV	R7,	#0						;R7= last
	  
	  LDR	R8,	=0x00FFFFFF				;R8=24bits
	  LDR	R2,=GPIO_PORTA_DATA_R
	  LDR	R3,	[R2]
	  AND	R3,R3,#0x10		;PA4
	  
	  LDR	R2,=GPIO_PORTE_DATA_R
	  LDR	R4,[R2]
	  AND	R4,R4,#0x0F		;PE0-3
	  
	  ADD	R5,	R3,	R4		;PA4, PE0-3	;R5 = Data goes into DataBuffer
	  LDR	R6,	=DataPt					;R6 = DataPt pointer
	  LDR	R2,[R6]
	  STRB	R5,[R2]			;stuff goes into data buffer
	  ADD	R2,	#1
	  STR	R2,[R6]
	  
	  LDR	R2,=NVIC_ST_CURRENT_R
	  LDR	R3,[R2]
	  LDR	R7,=Last
	  LDR	R1,[R7]
	  
	  SUB	R4,	R1,R3
	  AND	R4,	R4,R8
	  STR	R3,[R7]
	  LDR	R9, =TimePt					;R9 = TimePt
	  LDR	R2,[R9]
	  STR	R4,[R2]
	  ADD	R2, #4
	  STR	R2,[R9]
Full
	  POP  {R0-R9,PC}  

Heart	PUSH{R0,R1,LR}
		LDR	R0,=GPIO_PORTF_DATA_R 
		LDR	R1,[R0]
		EOR	R1,	#0x04	;PF2 ON
		STR	R1,[R0]
		POP{R0,R1,PC}
		
		
      
; edit the following only if you need to move pins from PA4, PE3-0      
; logic analyzer on the real board
PA4  equ  0x40004040   ; bit-specific addressing
PE30 equ  0x4002403C   ; bit-specific addressing
UART0_DR_R equ 0x4000C000 ;write to this to send data
SendDataToLogicAnalyzer
     LDR  R1,=PA4  
     LDR  R1,[R1]  ; read PA4
     LDR  R0,=PE30 ; read PE3-PE0
     LDR  R0,[R0]
     ORR  R0,R0,R1 ;combine into one 5-bit value
     ORR  R0,R0,#0x80 ;bit7=1 means digital data
     LDR  R1,=UART0_DR_R
     STR  R0,[R1] ; send data at 10 kHz
     BX   LR


     ALIGN    ; make sure the end of this section is aligned
     END      ; end of file

