// TinyCandle - ATtiny13A
//
// Tealight candle simulation for ATtiny13A.
// 
// The simulation code is based on the great work of Mark Sherman
// (https://github.com/carangil/candle).
//
// The lightweight pseudo random number generator based on 
// Galois linear feedback shift register is taken from Łukasz Podkalicki
// (https://blog.podkalicki.com/attiny13-pseudo-random-numbers/).
//
//
//                          +-\/-+
//        --- A0 (D5) PB5  1|    |8  Vcc
//        --- A3 (D3) PB3  2|    |7  PB2 (D2) A1 --- Button
// MOSFET --- A2 (D4) PB4  3|    |6  PB1 (D1) ------ LED3/4 PWM
//                    GND  4|    |5  PB0 (D0) ------ LED1/2 PWM
//                          +----+    
//
// Controller:  ATtiny13A
// Clockspeed:  1.2 MHz internal
//
// 2020 by Stefan Wagner 
// Project Files (EasyEDA): https://easyeda.com/wagiminator
// Project Files (Github):  https://github.com/wagiminator
// License: http://creativecommons.org/licenses/by-sa/3.0/


// less delay accuracy saves 16 bytes flash
#define __DELAY_BACKWARD_COMPATIBLE__ 1

// libraries
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// pin definitions
#define LED0          PB0
#define LED1          PB1
#define BUTTON        PB2
#define UNUSEDPIN     PB3
#define MOSFET        PB4

// candle simulation parameters
#define MINUNCALM     (20*256)
#define MAXUNCALM     (120*256)
#define UNCALMINC     20
#define MAXDEV        100
#define CANDLEDELAY   25

// some variables for the candle simulation
int16_t centerx   = MAXDEV;
int16_t centery   = MAXDEV/2;
int16_t xvel      = 0;
int16_t yvel      = 0;
uint16_t uncalm   = MINUNCALM;
int16_t uncalmdir = UNCALMINC;
uint8_t cnt       = 0;

// some variables for the random generator
uint16_t rn;

// pseudo random number generator
uint16_t prng(uint16_t maxvalue) {
  rn = (rn >> 0x01U) ^ (-(rn & 0x01U) & 0xB400U);
  return( (uint32_t)(maxvalue * rn) >> 16 );
}

// candle simulation
void updateCandle(void) {
  int movx = 0;
  int movy = 0;
   
  // random poke, intensity determined by uncalm value (0 is perfectly calm)
  movx = prng(uncalm >> 8) - (uncalm >> 9);
  movy = prng(uncalm >> 8) - (uncalm >> 9);
  
  // if reach most calm value, start moving towards uncalm
  if (uncalm < MINUNCALM) uncalmdir = UNCALMINC;
  
  // if reach most uncalm value, start going towards calm
  if (uncalm > MAXUNCALM) uncalmdir = -UNCALMINC;
  uncalm += uncalmdir;

  // move center of flame by the current velocity
  centerx += movx + (xvel >> 2);
  centery += movy + (yvel >> 2); 
  
  // range limits
  if (centerx < -MAXDEV) centerx = -MAXDEV;
  if (centerx >  MAXDEV) centerx =  MAXDEV;
  if (centery < -MAXDEV) centery = -MAXDEV; 
  if (centery >  MAXDEV) centery =  MAXDEV;

  // counter
  cnt++;
  if (! (cnt & 3)) {
    //attenuate velocity 1/4 clicks 
    xvel = (xvel * 999) / 1000;
    yvel = (yvel * 999) / 1000;
  }

  // apply acceleration towards center, proportional to distance from center (spring motion; hooke's law)
  xvel -= centerx;
  yvel -= centery;

  // set LEDs
  OCR0A = 155 + centerx;
  OCR0B = 155 + centery;
}

// main function
int main(void) {
  // PWM setup
  TCCR0A = (1<<COM0A1) | (1<<COM0B1)    // clear OC0A/OC0B on compare match, set at TOP
         | (1<<WGM01)  | (1<<WGM00);    // fast PWM 0x00 - 0xff
  TCCR0B = (1<<CS01);                   // set prescaler div8

  // setup pins
  DDRB   = (1<<LED0) | (1<<LED1)        // LED pins as output
         | (1<<MOSFET)                  // MOSFET pin as output
         | (1<<UNUSEDPIN);              // unused pin to output low to save power
  PORTB  = (1<<BUTTON) | (1<<MOSFET);   // pullup for button + MOSFET on

  // setup pin change interrupt
  GIMSK = (1<<PCIE);                    // turn on pin change interrupts
  PCMSK = (1<<BUTTON);                  // pin change interrupt on button pin
  sei();                                // enable global interrupts

  // disable unused peripherals to save power
  ADCSRA = 0;                           // disable ADC
  ACSR   = (1<<ACD);                    // disable analog comperator
  PRR    = (1<<PRADC);                  // shut down ADC
  set_sleep_mode (SLEEP_MODE_PWR_DOWN); // set sleep mode to power down

  // main loop
  while(1) {
    updateCandle();                     // candle simulation
    if (~PINB & (1<<BUTTON)) {          // if button is pressed
      DDRB  &= ~((1<<LED0) | (1<<LED1));// LED pins as input (PWM off)
      PORTB &= ~(1<<MOSFET);            // switch off MOSFET
      _delay_ms(10);                    // debounce button
      while (~PINB & (1<<BUTTON));      // wait for button released
      _delay_ms(10);                    // debounce button
      sleep_mode();                     // sleep until button pressed
      DDRB  |= (1<<LED0) | (1<<LED1);   // LED pins as output (PWM on)
      PORTB |= (1<<MOSFET);             // switch on MOSFET
      _delay_ms(10);                    // debounce button
      while (~PINB & (1<<BUTTON));      // wait for button released
    }  
  _delay_ms(CANDLEDELAY);               // delay
  }
}

// pin change interrupt service routine
EMPTY_INTERRUPT (PCINT0_vect);          // nothing to be done here, just wake up from sleep
