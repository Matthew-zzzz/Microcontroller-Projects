/*
 * File:   main.c
 * Author: Joseph Tran, Matthew Zalavarria
 *
 * Lab 5 Part 2 - Two-Digit Seven-Segment Seconds Counter (Common Anode)
 * ECE 3301L - Introduction to Microcontrollers Laboratory
 *
 * Description:
 *   Counts from 00 to 59, then resets to 00.
 *   Uses two common-anode 7-segment displays with digit multiplexing.
 *
 * Pin Assignments:
 *   PORTB (RB0-RB6) - Segments a-g for DIGIT 1 (tens)
 *     RB0=a, RB1=b, RB2=c, RB3=d, RB4=e, RB5=f, RB6=g
 *   PORTC (RC0-RC6) - Segments a-g for DIGIT 2 (ones)
 *     RC0=a, RC1=b, RC2=c, RC3=d, RC4=e, RC5=f, RC6=g
 *   RD4 - Digit 1 enable (HIGH = ON, through 1K to COM pin)
 *   RD5 - Digit 2 enable (HIGH = ON, through 1K to COM pin)
 *
 * Wiring:
 *   RD4 - [1K] - Digit 1 COM (anode)
 *   RD5 - [1K] - Digit 2 COM (anode)
 *   Segment pins - [330 ohm] - RBx / RCx
 *
 * Multiplexing:
 *   Rapidly alternate between digit 1 and digit 2 (~5 ms each).
 *   100 multiplex cycles of ~10 ms each = ~1 second.
 */

#include <xc.h>
#include <stdint.h>
#include "PIC18F46K22-Config.h"

#define _XTAL_FREQ 16000000UL   // 16 MHz HFINTOSC

/* 7-Segment Patterns (Common Anode: 0=ON, 1=OFF)
 *   Bit mapping: bit6=g, bit5=f, bit4=e, bit3=d, bit2=c, bit1=b, bit0=a
 * Digit 0 is done for you - work out the other nine. */
static const uint8_t sevenSeg[10] = { // CATHODE
    0x3F, // 0: a,b,c,d,e,f;
    0x06, // 1: b,c
    0x5B, // 2: a,b,d,e,g
    0x4F, // 3: a,b,c,d,g
    0x66, // 4: b,c,f,g
    0x6D, // 5: a,c,d,f,g
    0x7D, // 6: a,c,d,e,f,g
    0x07, // 7: a,b,c
    0x7F, // 8: all segments
    0x6F  // 9: a,b,c,d,f,g
};

#define SEG_BLANK 0x00  /* All segments OFF */

static void init(void) {
    // TODO: 16 MHz HFINTOSC
    OSCCONbits.IRCF = 0b111; // 16 MHZ
    OSCCONbits.SCS = 0b10; // INTERNAL block
    
    // TODO: ANSELB/C/D = digital
    ANSELB = 0;
    ANSELC = 0;
    ANSELD = 0;
    
    // TODO: RB0-RB6 outputs (preserve RB7), latches cleared
    TRISB &= 0x80;
    LATB &= 0x80;
    
    // TODO: RC0-RC6 outputs (preserve RC7), latches cleared
    TRISC &= 0x80;
    LATC &= 0x80;
    
    // TODO: RD4, RD5 outputs, both digits OFF initially
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 0;
    
    LATDbits.LATD4 = 0;
    LATDbits.LATD5 = 0;
}

/**
 * Multiplex both digits for approximately 1 second.
 *
 * Each cycle: ~5 ms digit 1 + ~5 ms digit 2 = ~10 ms.
 * 100 cycles = ~1 second.
 */
static void display_one_second(uint8_t tens, uint8_t ones) {
    // TODO: for 100 cycles:
    //   1. Turn digit 2 enable OFF
    //   2. Put the tens pattern on PORTB (preserve RB7)
    //   3. Turn digit 1 enable ON, wait ~5 ms
    //   4. Turn digit 1 enable OFF
    //   5. Put the ones pattern on PORTC (preserve RC7)
    //   6. Turn digit 2 enable ON, wait ~5 ms
    // Afterwards turn both digits off to prevent ghosting.
    uint8_t i;
    for (i = 0; i < 100; i ++) {
        LATDbits.LATD5 = 0; // digit 2 off
        LATB = (LATB & 0x80 | (sevenSeg[tens] & 0x7F)); //output tens on portB
        
        LATDbits.LATD4 = 1; // digit 1 on
        __delay_ms(5);
        
        LATDbits.LATD4 = 0; // digit 1 off
        
        LATC = (LATC & 0x80) | (sevenSeg[ones] & 0x7F); //output ones pattern on Portc 
        
        LATDbits.LATD5 = 1; // digit 2 ON
        __delay_ms(5);
    } 
  // (void)tens;
   //(void)ones;
}

void main(void) {
    init();

    uint8_t seconds = 0;
    uint8_t tens;
    uint8_t ones;

    init();
    
    while (1) {
        // TODO: split seconds into tens and ones digits,
        //       call display_one_second(), then increment seconds
        //       and wrap 59 -> 0
        tens = seconds / 10;
        ones = seconds % 10;
        
        display_one_second(tens,ones);
        
        seconds++;
        
        if (seconds >= 60) {
            seconds = 0;
        }
      //  (void)seconds;
    }
}