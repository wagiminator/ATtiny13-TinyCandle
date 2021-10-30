// ===================================================================================
// Project:   TinyCandle - Tealight Candle Simulation based on ATtiny13A
// Version:   v1.0
// Year:      2020
// Author:    Stefan Wagner
// Github:    https://github.com/wagiminator
// EasyEDA:   https://easyeda.com/wagiminator
// License:   http://creativecommons.org/licenses/by-sa/3.0/
// ===================================================================================
//
// Description:
// ------------
// Simple tealight candle simulation with four LEDs for ATtiny13A.
//
// References:
// -----------
// The simulation code is based on the great work of Mark Sherman.
// https://github.com/carangil/candle
//
// The lightweight pseudo random number generator based on 
// Galois linear feedback shift register is taken from Łukasz Podkalicki.
// https://blog.podkalicki.com/attiny13-pseudo-random-numbers/
//
// Wiring:
// -------
//                           +-\/-+
//        --- RST ADC0 PB5  1|°   |8  Vcc
//        ------- ADC3 PB3  2|    |7  PB2 ADC1 -------- Button
// MOSFET ------- ADC2 PB4  3|    |6  PB1 AIN1 OC0B --- LED3/4 PWM
//                     GND  4|    |5  PB0 AIN0 OC0A --- LED1/2 PWM
//                           +----+
//
// Compilation Settings:
// ---------------------
// Controller:  ATtiny13A
// Core:        MicroCore (https://github.com/MCUdude/MicroCore)
// Clockspeed:  1.2 MHz internal
// BOD:         BOD disabled
// Timing:      Micros disabled
//
// Leave the rest on default settings. Don't forget to "Burn bootloader"!
// No Arduino core functions or libraries are used. Use the makefile if 
// you want to compile without Arduino IDE.
//
// Fuse settings: -U lfuse:w:0x2a:m -U hfuse:w:0xff:m


// ===================================================================================
// Libraries and Definitions
// ===================================================================================

// Libraries
#include <avr/io.h>               // for GPIO
#include <avr/sleep.h>            // for sleep modes
#include <avr/interrupt.h>        // for interrupts
#include <util/delay.h>           // for delays

// Pin definitions
#define LED0          PB0         // pin connected to LED 1/2
#define LED1          PB1         // pin connected to LED 3/4
#define BUTTON        PB2         // pin connected to button
#define UNUSEDPIN     PB3         // unused pin
#define MOSFET        PB4         // pin connected to MOSFET

// Less delay accuracy saves 16 bytes flash
#define __DELAY_BACKWARD_COMPATIBLE__ 1

// ===================================================================================
// Pseudo Random Number Generator (adapted from Łukasz Podkalicki)
// ===================================================================================

// Start state (any nonzero value will work)
uint16_t rn = 0xACE1;

// Pseudo random number generator
uint16_t prng(uint16_t maxvalue) {
  rn = (rn >> 0x01) ^ (-(rn & 0x01) & 0xB400);
  return(rn % maxvalue);
}

// ===================================================================================
// Candle Simulation Implementation (adapted from Mark Sherman)
// ===================================================================================

// Candle simulation parameters
#define MINUNCALM     ( 5 * 256)
#define MAXUNCALM     (60 * 256)
#define UNCALMINC     10
#define MAXDEV        100
#define CANDLEDELAY   15

// Some variables
int16_t centerx = MAXDEV;
int16_t centery = MAXDEV / 2;
int16_t xvel = 0;
int16_t yvel = 0;
uint16_t uncalm =   MINUNCALM;
int16_t uncalmdir = UNCALMINC;
uint8_t cnt = 0;

// Candle simulation
void updateCandle() {
  int16_t movx=0;
  int16_t movy=0;
    
  // Random trigger brightness oscillation, if at least half uncalm
  if(uncalm > (MAXUNCALM / 2)) {
    if(prng(2000) < 5) uncalm = MAXUNCALM * 2;  //occasional 'bonus' wind
  }
   
  // Random poke, intensity determined by uncalm value (0 is perfectly calm)
  movx = prng(uncalm >> 8) - (uncalm >> 9);
  movy = prng(uncalm >> 8) - (uncalm >> 9);
  
  // If reach most calm value, start moving towards uncalm
  if(uncalm < MINUNCALM) uncalmdir =  UNCALMINC;
  
  // If reach most uncalm value, start going towards calm
  if(uncalm > MAXUNCALM) uncalmdir = -UNCALMINC;
  uncalm += uncalmdir;

  // Move center of flame by the current velocity
  centerx += movx + (xvel >> 2);
  centery += movy + (yvel >> 2); 
  
  // Range limits
  if(centerx < -MAXDEV) centerx = -MAXDEV;
  if(centerx >  MAXDEV) centerx =  MAXDEV;
  if(centery < -MAXDEV) centery = -MAXDEV; 
  if(centery >  MAXDEV) centery =  MAXDEV;

  // Counter
  cnt++;
  if(!(cnt & 3)) {
    // Attenuate velocity 1/4 clicks 
    xvel = (xvel * 999) / 1000;
    yvel = (yvel * 999) / 1000;
  }

  // Apply acceleration towards center, proportional to distance from center
  // (spring motion; hooke's law)
  xvel -= centerx;
  yvel -= centery;

  // Set LEDs
  OCR0A = 128 + centerx;
  OCR0B = 128 + centery;
}

// ===================================================================================
// Main Function
// ===================================================================================

// Main function
int main(void) {
  // PWM setup
  TCCR0A = (1<<COM0A1) | (1<<COM0B1)    // clear OC0A/OC0B on compare match, set at TOP
         | (1<<WGM01)  | (1<<WGM00);    // fast PWM 0x00 - 0xff
  TCCR0B = (1<<CS00);                   // start timer without prescaler

  // Setup pins
  DDRB   = (1<<LED0) | (1<<LED1)        // LED pins as output
         | (1<<MOSFET)                  // MOSFET pin as output
         | (1<<UNUSEDPIN);              // unused pin to output low to save power
  PORTB  = (1<<BUTTON) | (1<<MOSFET);   // pullup for button + MOSFET on

  // Setup pin change interrupt
  GIMSK = (1<<PCIE);                    // turn on pin change interrupts
  PCMSK = (1<<BUTTON);                  // pin change interrupt on button pin
  sei();                                // enable global interrupts

  // Disable unused peripherals to save power
  ADCSRA = 0;                           // disable ADC
  ACSR   = (1<<ACD);                    // disable analog comperator
  PRR    = (1<<PRADC);                  // shut down ADC
  set_sleep_mode (SLEEP_MODE_PWR_DOWN); // set sleep mode to power down

  // Main loop
  while(1) {
    updateCandle();                     // candle simulation
    if(~PINB & (1<<BUTTON)) {           // if button is pressed
      DDRB  &= ~((1<<LED0) | (1<<LED1));// LED pins as input (PWM off)
      PORTB &= ~(1<<MOSFET);            // switch off MOSFET
      _delay_ms(10);                    // debounce button
      while(~PINB & (1<<BUTTON));       // wait for button released
      _delay_ms(10);                    // debounce button
      sleep_mode();                     // sleep until button pressed
      DDRB  |= (1<<LED0) | (1<<LED1);   // LED pins as output (PWM on)
      PORTB |= (1<<MOSFET);             // switch on MOSFET
      _delay_ms(10);                    // debounce button
      while(~PINB & (1<<BUTTON));       // wait for button released
    }  
  _delay_ms(CANDLEDELAY);               // delay
  }
}

// Pin change interrupt service routine
EMPTY_INTERRUPT(PCINT0_vect);           // nothing to be done here, just wake up from sleep
