/*
 * File:   lab8.c
 * Author: Joseph Tran, Matthew Zalavarria
 *
 * Lab 8 - DHT11 Humidity and Temperature Sensor
 * ECE 3301L - Introduction to Microcontrollers Laboratory
 *
 * Description:
 *   Reads humidity and temperature from a DHT11 sensor and displays the
 *   values on a 16x2 LCD, using the single-wire DHT11 protocol with
 *   timing-based bit decoding.
 *
 * Pin Assignments:
 *   RD0        - LCD RS
 *   RD2        - LCD EN
 *   RA4-RA7    - LCD D4-D7
 *   RD1        - DHT11 data line (with 10K pull-up to VDD)
 *
 * DHT11 Wiring:
 *   Pin 1 (VDD)  - 5V (or 3.3V)
 *   Pin 2 (DATA) - RD1 + 10K pull-up to VDD
 *   Pin 3 (NC)   - not connected
 *   Pin 4 (GND)  - GND
 *
 * DHT11 Protocol (40 bits total, MSB first):
 *   Byte 1: Humidity integer
 *   Byte 2: Humidity decimal (always 0 for DHT11)
 *   Byte 3: Temperature integer
 *   Byte 4: Temperature decimal (always 0 for DHT11)
 *   Byte 5: Checksum (sum of bytes 1-4, lower 8 bits)
 *
 * LCD driver (LCD.c / LCD.h) is PROVIDED - do not modify LCD.c.
 */

#include <xc.h>
#include <stdint.h>
#include "PIC18F46K22-Config.h"
#include "LCD.h"

//#define _XTAL_FREQ 16000000UL   // 16 MHz HFINTOSC

/* DHT11 Pin Definitions */
#define DHT11_TRIS  TRISDbits.TRISD1
#define DHT11_LAT   LATDbits.LATD1
#define DHT11_PORT  PORTDbits.RD1

/* DHT11 Data Storage: humI, humD, tempI, tempD, checksum */
static uint8_t dht11_data[5];

static uint8_t DHT11_wait_while(uint8_t state, uint16_t timeout_us)
{
    while (DHT11_PORT == state)
    {
        if (timeout_us == 0)
        {
            return 0;
        }

        timeout_us--;
        __delay_us(1);
    }

    return 1;
}

static void init(void) {
    // TODO: 16 MHz HFINTOSC
    // TODO: ANSELA / ANSELD digital (LCD + DHT11 pins)
    OSCCON = 0b01110010;

    OSCTUNEbits.PLLEN = 0;
    while (OSCCONbits.HFIOFS == 0)
    {
    }
    
    
    ANSELA = 0;
    ANSELD = 0; // digital
    
    DHT11_LAT = 1;
    DHT11_TRIS = 1; // release DHT11 data line. set HIGH
}

/**
 * Send the start signal and read 40 bits from the DHT11.
 * Returns 1 on success (checksum valid), 0 on failure.
 */
static uint8_t DHT11_read(void) {
    uint8_t byte;
    uint8_t bit;
    uint8_t sum;
    // TODO - implement the DHT11 protocol:
    //
    for (byte = 0; byte < 5; byte++)
    // Step 1: MCU start signal
    //   - Drive the data line LOW (output mode) for at least 18 ms
    //   - Release: drive HIGH briefly (~30 us), then switch to INPUT
    //     and let the pull-up hold the line HIGH
    //
        DHT11_TRIS = 0;
    DHT11_LAT = 0;
    __delay_ms(22);
    
       DHT11_LAT = 1; // briefly drives HIGH
    __delay_us(35);
    DHT11_TRIS = 1;
    
    // Step 2: Sensor response (use timeouts so a missing sensor
    //         cannot hang the program!)
    //   - Wait out: line HIGH -> LOW  (~80 us response low)
    //   - Wait out: LOW -> HIGH      (~80 us response high)
    //   - Wait out: HIGH -> LOW      (start of first bit)
    //
    if (!DHT11_wait_while(1, 120)) 
    {
        return 0;
    }
    
    if (!DHT11_wait_while(0,120))
    {
        return 0;
    }
    
    if (!DHT11_wait_while(1,120))
    {
        return 0;
    }
    
   
    // Step 3: Read 40 bits (5 bytes, MSB first). For each bit:
    //   - Wait through the ~50 us LOW bit-start
    //   - Delay ~35 us, then sample the line:
    //       still HIGH -> bit is '1' (then wait for the HIGH to end)
    //       already LOW -> bit is '0'
    //   - Shift the bit into dht11_data[i]
    //
     for (byte = 0; byte < 5; byte++)
    {
        for (bit = 0; bit < 8; bit++)
        {
            if (!DHT11_wait_while(0, 100))
            {
                return 0;
            }
            dht11_data[byte] <<= 1;
            
            __delay_us(35);
            
            if (DHT11_PORT == 1)
            {
                dht11_data[byte] |= 0x01;
                
                if (!DHT11_wait_while(1, 100))
                {
                    return 0;
                }
                }
            }
        }
    
    // Step 4: Verify the checksum:
    //   (bytes 0+1+2+3) & 0xFF must equal byte 4
    //
    // Return 1 only if everything succeeded.
     sum = (uint8_t)(
        dht11_data[0] +
        dht11_data[1] +
        dht11_data[2] +
        dht11_data[3]
    );

    if (sum != dht11_data[4])
    {
        return 0;
    }

    return 1;
}


void main(void) {
    init();
    LCD_init();

    // TODO: Write static labels:
    //   line 1: "Temp:       C"
    //   line 2: "Hum :       %"

    LCD_cursor_set(1, 1);
    LCD_write_string("Temp:       C");
    
    LCD_cursor_set(2, 1);
    LCD_write_string("Hum:       %");
    // DHT11 needs ~1 second to stabilize after power-on
    __delay_ms(1000);

    while (1) {
        // TODO: if DHT11_read() succeeds:
        //   - print dht11_data[2] (temperature integer) on line 1
        //   - print dht11_data[0] (humidity integer) on line 2
        // else show an error indicator on the LCD.
        //
        // The DHT11 requires at least 1 second between readings -
        // delay 2 seconds per loop.
          if (DHT11_read())
        {
            /* Temperature integer byte */
            LCD_cursor_set(1, 7);
            LCD_write_variable((int32_t)dht11_data[2], 3);

            /* Humidity integer byte */
            LCD_cursor_set(2, 7);
            LCD_write_variable((int32_t)dht11_data[0], 3);
        }
        else
        {
            /* Timeout or checksum error */
            LCD_cursor_set(1, 7);
            LCD_write_string("ERR");

            LCD_cursor_set(2, 7);
            LCD_write_string("ERR");
        }

        /* Wait two seconds before requesting another reading */
        __delay_ms(2000);
    }
}

