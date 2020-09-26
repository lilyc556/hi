; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

DIGIT		EQU			0
	
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
		PUSH	{R4,LR}
		SUB	SP, #8				;allocating
		MOV	R2, #10		

start	CMP	R0,	#10
		BHS	recur
		ADD	R0,	#0x30			;base case
		BL	ST7735_OutChar
		B	done
		
recur	MOV	R1, R0				;R1 = R0 = 654
		UDIV R0, R0, R2			;R0 = 654/10 = 65
		MUL	R3, R0, R2			;R3 = 65 * 10 = 650
		SUB	R3, R1, R3 			;R3 = 654-650 = 4 
		ADD	R3, #0x30
		STRB R3, [SP,#DIGIT]	;access	
		BL	LCD_OutDec			;OBJECTIVE SOMEHOW BL start
		LDRB R0, [SP,#DIGIT]
		BL	ST7735_OutChar
		
done	ADD	SP,	#8				;deallocating
		POP	{R4,LR}
		BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix 
		MOV	R12, #0				;R4 = . position counter			;need to move this
LCD_Fix
		PUSH	{R4,LR}
		SUB	SP, #8				;allocating
		MOV	R2, #10
		
		CMP	R0, #1000
		BLO	start2
		MOV	R0, #0x2A			;R0 = ASCII '*'
		BL	ST7735_OutChar
		MOV	R0,	#0x2E
		BL	ST7735_OutChar
		MOV	R0, #0x2A
		BL	ST7735_OutChar
		MOV	R0,	#0x2A
		BL	ST7735_OutChar
		B	done2
		

start2	CMP	R12, #2
		BLO	recur2
		ADD	R0,	#0x30			;base case
		BL	ST7735_OutChar
		B	done2
		
recur2	MOV	R1, R0				;R1 = R0 = 654
		UDIV R0, R0, R2			;R0 = 654/10 = 65
		MUL	R3, R0, R2			;R3 = 65 * 10 = 650
		SUB	R3, R1, R3 			;R3 = 654-650 = 4 
		ADD	R3, #0x30
		STRB R3, [SP,#DIGIT]	;access
		ADD	R12, #1	
		BL	LCD_Fix
		CMP	R12, #2
		BEQ	dot
back	LDRB R0, [SP,#DIGIT]
		BL	ST7735_OutChar

		
done2	ADD	SP,	#8				;deallocating
		POP	{R4,LR}
		BX  LR

dot		MOV	R0, #0x2E			;R0 = ASCII '.'
		BL	ST7735_OutChar
		SUB	R12, #1				;making everything after R12 glitchy
		B	back
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
