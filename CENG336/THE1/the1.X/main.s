PROCESSOR 18F8722

#include <xc.inc>

; CONFIGURATION (DO NOT EDIT)
; CONFIG1H
CONFIG OSC = HSPLL      ; Oscillator Selection bits (HS oscillator, PLL enabled (Clock Frequency = 4 x FOSC1))
CONFIG FCMEN = OFF      ; Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
CONFIG IESO = OFF       ; Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)
; CONFIG2L
CONFIG PWRT = OFF       ; Power-up Timer Enable bit (PWRT disabled)
CONFIG BOREN = OFF      ; Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
; CONFIG2H
CONFIG WDT = OFF        ; Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
; CONFIG3H
CONFIG LPT1OSC = OFF    ; Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
CONFIG MCLRE = ON       ; MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)
; CONFIG4L
CONFIG LVP = OFF        ; Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
CONFIG XINST = OFF      ; Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))
CONFIG DEBUG = OFF      ; Disable In-Circuit Debugger


GLOBAL var1
GLOBAL var2
GLOBAL var3
GLOBAL re0state
GLOBAL re0task
GLOBAL re1state
GLOBAL re1task
GLOBAL result
GLOBAL counter
GLOBAL counter2

; Define space for the variables in RAM
PSECT udata_acs
var1:
    DS 1 ; Allocate 1 byte for var1
var2:
    DS 1 
var3:
    DS 1
re0state:
    DS 1
re0task:
    DS 1
re1state:
    DS 1
re1task:
    DS 1
counter:
    DS 1 
counter2:
    DS 1 
temp_result:
    DS 1   
result: 
    DS 1


PSECT resetVec,class=CODE,reloc=2
resetVec:
    goto       main

PSECT CODE
main:
    clrf var1	; var1 = 0		
    clrf var2   ; var2 = 0
    clrf var3
    clrf re0state
    clrf re0task
    clrf re1state
    clrf re1task
    clrf result ; result = 0
    
    clrf counter
    movlw 197
    movwf counter2
    
;    setf counter
;    movlw 60
;    movwf counter2

    ; PORTB
    ; LATB
    ; TRISB determines whether the port is input or output (=1 is input)
    
    clrf TRISB
    clrf TRISC
    clrf TRISD
    setf TRISE ; PORTE is input, by default the input pin will be zero, hoca böyle söylüyo
    
    ; light up all pins in PORTB PORTC and PORTD
    setf PORTB
    setf LATC 
    setf LATD
   
    ; we are busy waiting for 1 second
    call busy_wait
    
    ; after waiting we turned off all the LEDs
    clrf PORTB
    clrf LATC
    clrf LATD
    
main_loop:
    ; round robin approach
    call check_buttons
    call update_display
    goto main_loop
    
busy_wait: ; waits for 1 second
    movlw 250
    movwf var3
    second_outer_loop_start:
	movlw 38
	movwf var2 ; var2 = 0
	first_outer_loop_start:
	    ; for(var1 = 255 ; var != 0 ; --var1) 
	    setf var1 ; this sets var1 = 255 set all bits to 1 in binary representation
	    loop_start:
		decf var1
		bnz loop_start
	    incfsz var2 
	    goto first_outer_loop_start ; we can use bra instead of goto
	incfsz var3
	goto second_outer_loop_start
    return
    
;busy_wait:
;    movlw 10
;    movwf var3
;    third_loop:
;	movlw 128
;	movwf var2
;	second_loop:
;	    setf var1
;	    first_loop:
;		decfsz var1
;		goto first_loop
;	    decfsz var2
;	    goto second_loop
;	decfsz var3
;	goto third_loop
;    return

check_re0:
    movlw 1
    andwf re0state
    bz re0_state_zero
    bra re0_state_notzero
    re0_state_zero:
	btfss PORTE, 0 ; check RE0, while it is 1 skip the next instruction
	bra check_re0_finish ; re0_state = 0 && nobody pressing the button
	movlw 1
	movwf re0state
	bra check_re0_finish
    re0_state_notzero:
	btfsc PORTE, 0 ; check RE0, while it is 0 skip the next instruction
	bra check_re0_finish ; re0_state = 1 && somebody pressing the button
	movlw 0
	movwf re0state
	; start doing something in update display by using the re0task flag
	;movlw 1
	;movwf re0task
	movlw 1
	andwf re0task
	bz re0task_will_be_enabled
	re0task_will_be_disabled:
	    movlw 0
	    movwf re0task
	    bra check_re0_finish
	re0task_will_be_enabled:
	    movlw 1
	    movwf re0task
    check_re0_finish:
	return
	
check_re1:
    movlw 1
    andwf re1state
    bz re1_state_zero
    bra re1_state_notzero
    re1_state_zero:
	btfss PORTE, 1 ; check RE0, while it is 1 skip the next instruction
	bra check_re1_finish ; re0_state = 0 && nobody pressing the button
	movlw 1
	movwf re1state
	bra check_re1_finish
    re1_state_notzero:
	btfsc PORTE, 1 ; check RE0, while it is 0 skip the next instruction
	bra check_re1_finish ; re0_state = 1 && somebody pressing the button
	movlw 0
	movwf re1state
	; start doing something in update display by using the re0task flag
	;movlw 1
	;movwf re0task
	movlw 1
	andwf re1task
	bz re1task_will_be_enabled
	re1task_will_be_disabled:
	    movlw 0
	    movwf re1task
	    bra check_re1_finish
	re1task_will_be_enabled:
	    movlw 1
	    movwf re1task
    check_re1_finish:
	return
    
check_buttons:
    call check_re0 ; relative call
    call check_re1
    return

portb_progress_bar:
    ; do modification
    clrf WREG
    subwf PORTB, 0
    bz case_0
    movlw 00000001B
    subwf PORTB, 0
    bz case_1
    movlw 00000011B
    subwf PORTB, 0
    bz case_2
    movlw 00000111B
    subwf PORTB, 0
    bz case_3
    movlw 00001111B
    subwf PORTB, 0
    bz case_4
    movlw 00011111B
    subwf PORTB, 0
    bz case_5
    movlw 00111111B
    subwf PORTB, 0
    bz case_6
    movlw 01111111B
    subwf PORTB, 0
    bz case_7
    movlw 11111111B
    subwf PORTB, 0
    bz case_8
    case_0:
	bsf PORTB, 0
	return
    case_1:
	bsf PORTB, 1
	return
    case_2:
	bsf PORTB, 2
	return
    case_3:
	bsf PORTB, 3
	return
    case_4:
	bsf PORTB, 4
	return
    case_5:
	bsf PORTB, 5
	return
    case_6:
	bsf PORTB, 6
	return
    case_7:
	bsf PORTB, 7
	return
    case_8:
	clrf PORTB
	return
	
portc_progress_bar:
    ; do modification
    clrf WREG
    subwf PORTC, 0
    bz case_9
    movlw 10000000B
    subwf PORTC, 0
    bz case_10
    movlw 11000000B
    subwf PORTC, 0
    bz case_11
    movlw 11100000B
    subwf PORTC, 0
    bz case_12
    movlw 11110000B
    subwf PORTC, 0
    bz case_13
    movlw 11111000B
    subwf PORTC, 0
    bz case_14
    movlw 11111100B
    subwf PORTC, 0
    bz case_15
    movlw 11111110B
    subwf PORTC, 0
    bz case_16
    movlw 11111111B
    subwf PORTC, 0
    bz case_17
    case_9:
	bsf PORTC, 7
	return
    case_10:
	bsf PORTC, 6
	return
    case_11:
	bsf PORTC, 5
	return
    case_12:
	bsf PORTC, 4
	return
    case_13:
	bsf PORTC, 3
	return
    case_14:
	bsf PORTC, 2
	return
    case_15:
	bsf PORTC, 1
	return
    case_16:
	bsf PORTC, 0
	return
    case_17:
	clrf PORTC
	return
    
re0_task:
    movlw 1
    andwf re0task
    bz re0_task_finish ; if re0task flag is 0 than we do nothing
    ; else update the progress bar of PORTC as expected
    re0_task_continues:
	call portc_progress_bar
	return
    re0_task_finish:
	clrf PORTC
	return
	
re1_task:
    movlw 1
    andwf re1task
    bz re1_task_finish ; if re0task flag is 0 than we do nothing
    ; else update the progress bar of PORTC as expected
    re1_task_continues:
	call portb_progress_bar
	return
    re1_task_finish:
	clrf PORTB
	return
    
update_display:
    incfsz counter
    return
    counter_overflowed:
	incfsz counter2
	return
	counter2_overflowed:
	    btg PORTD, 0
	    call re0_task
	    call re1_task
	    movlw 198
	    ;movwf counter
	    movwf counter2
	    return
	
;update_display:
;    decfsz counter
;    return
;    counter_underflowed:
;	decfsz counter2
;	return
;	counter2_underflowed:
;	    btg PORTD, 0
;	    call re0_task
;	    call re1_task
;	    movlw 60
;	    movwf counter2
;	    return
    
end resetVec