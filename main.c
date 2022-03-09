/*
 * File:   main.c
 * Author: George.Nikolaidis
 *
 * Created on Mar 09, 2022, 18:00 PM
 */


// PIC12F615 Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config IOSCFS = 8MHZ    // Internal Oscillator Frequency Select (8 MHz)
#pragma config BOREN = OFF      // Brown-out Reset Selection bits (BOR disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#define _XTAL_FREQ 8000000
#define DISABLE_PWM_Service()               (CCP1CON = 0x0)
#define ENABLE_DIGITAL_IO_Pins()            (ANSEL = 0X0)
#define LED_Toggle()             	    do { GPIObits.GP5 = ~GPIObits.GP5; } while(0)
#define TMR1_CLEAR_FLAG_INT()               (PIR1bits.TMR1IF = 0x0)
#define ENABLE_TMR1_INT()                   (PIE1bits.TMR1IE = 0x1)
#define TMR1_ON()                           (T1CONbits.TMR1ON = 0x1)
#define TMR1_OFF()                          (T1CONbits.TMR1ON = 0x0)
#define FOSC4_TMR1_CLOCK()                  (CMCON1bits.T1ACS = 0x0)
#define CLEAR_TMR1H()                       (TMR1H = 0x0)
#define CLEAR_TMR1L()                       (TMR1L = 0x0)
#define ALL_GPIO_AS_OUTPUTS()               (TRISA = 0x8)

void settmr1hl(){
    TMR1H = 0x3C;
    TMR1L = 0xAF;
}

void SYSTEM_Initialize(){
    /* CONFIGURATION OF GPIO
     * GP1, GP2, GP4, GP5 as OUTPUT
     * GP3 - as INPUT
     * GP5 - OUTPUT LED - PIN 2 */
    
    /* Formula to calculate: 
       Timer 1 Clock Source is Instruction Clock (FOSC\4) is been used
       with prescaler 1:1 
       Sys_Clock = FOSC/4 = 8Mhz/4 = 2*10^6 
       1/Sys_Clock = 500ns or 5*10^-7 
       For 20hz square wave, 1/20 = 0.05Sec or 50mSec period 
       for 50% duty cycle, need a pulse every 25mSec
       TMR1 is a 16bit wide timer, the TMR1H_TMR1L is:
       TMR1H_TMR1L = 0.025 / 5*10^-7 = 50000  
       TMR1 rolls over on 0xFFFF - 65535 so, 65535 - 50000 = 15535
       TMR1H_TMR1L = 15535 or 00111100 10101111 the first nible goes to
       TMR1H and the second one to TMR1L to achieve a timer1 interrupt
       every 25mSec */

    ENABLE_DIGITAL_IO_Pins();/* disable analog enable digital pins */
    ALL_GPIO_AS_OUTPUTS();   /* 00001000 all OUTPUT, except GP3 as INPUT */
    GPIObits.GP0 = 0x0;      /*  make all GP output low except GP3 */
    GPIObits.GP1 = 0x0;
    GPIObits.GP2 = 0x0;
    GPIObits.GP4 = 0x0;
    GPIObits.GP5 = 0x0;

    OPTION_REG   = 0x80;  /* 10000000,
                           * GPIO pull-ups disable0,
                           * INTEDG on rising     0, 
                           * TOSC FOSC/4 TOSE     0, 
                           * PSA                  0, 
                           * PS                   000 1:2 */
    WPU = 0x0;            /* Disable weak pull ups  */
    IOC = 0x0;            /* Interrupt on change disabled */ 
    INTCON = 0xC0;        /* 11000000
                           * GIE  1 Global Interrupt Enable bit
                           * PEIE 1 Enables all unmasked interrupts
                           * TOIE 0 Timer0 Overflow Interrupt Enable bit
                           * INTE 0 GP2/INT External Interrupt Enable bit
                           * GPIE 0 GPIO Change Interrupt Enable bit, IOC must EN
                           * TOIF 0 Timer0 Overflow Interrupt Flag bit(2)
                           * INTF 0 GP2/INT External Interrupt Flag bit
                           * GPIF 0 GPIO Change Interrupt Flag bit */
    DISABLE_PWM_Service();
    TMR1_CLEAR_FLAG_INT();/* Make sure INT FLAG is cleared */
    TMR1H = 0x3C;         /* Set 15535 in the HL TMR1 for 25mSec interrupt */
    TMR1L = 0xAF;         /* 50000 * 5*10^-7 = 0.025Sec or 25mSec */
    T1CON = 0x4;          /* T1CKPS1:T1CKPS0: 1:1 0
                             T1OSCEN OFF 0
                             T1SYNC ignored if TMR1CS = 0
                             TMR1CS 0 Internal clock (FOSC/4)
                             TMR1ON 0 currently stopped.*/
    FOSC4_TMR1_CLOCK();   /* 0 = Timer 1 Clock Source is Instruction Clock (FOSC\4)*/
    ENABLE_TMR1_INT();    /* Enable Overflow Interrupt Enable bit */
    TMR1_ON();            /* TMR1 STARTS */
}

void __interrupt() ISR (void){
    if(PIR1bits.TMR1IF){
        LED_Toggle();
        settmr1hl();
        TMR1_CLEAR_FLAG_INT();
    }
}

void main(void) {
    SYSTEM_Initialize();
    while(1);
}

