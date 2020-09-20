// TinyCandle
//
// Tealight candle simulation for ATtiny13a.
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
// Controller:  ATtiny13
// Core:        MicroCore (https://github.com/MCUdude/MicroCore)
// Clockspeed:  1.2 MHz internal
// BOD:         disabled (to save energy)
// Timing:      Micros disabled
//
// 2020 by Stefan Wagner (https://easyeda.com/wagiminator)
// License: http://creativecommons.org/licenses/by-sa/3.0/


// Libraries
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>

// Pin definiations
#define LED0          PB0
#define LED1          PB1
#define BUTTON        PB2
#define MOSFET        PB4

// Candle simulation parameters
#define MINUNCALM     (20*256)
#define MAXUNCALM     (120*256)
#define UNCALMINC     20
#define MAXDEV        100
#define CANDLEDELAY   25

// some variables for the candle simulation
int centerx = MAXDEV;
int centery = MAXDEV/2;
int xvel    = 0;
int yvel    = 0;
uint16_t uncalm  = MINUNCALM;
int uncalmdir= UNCALMINC;
uint8_t cnt=0;

// some variables for the random generator
uint16_t rn;


void setup() {
  // PWM setup
  TCCR0A = bit(COM0A1) | bit(COM0B1) | bit(WGM01) | bit(WGM00);
  TCCR0B = bit(CS01);

  // Setup pins
  DDRB   = bit(LED0) | bit(LED1) | bit(MOSFET); // set output pins
  PORTB |= bit(BUTTON) | bit(MOSFET);           // pullup for button + MOSFET on

  // setup pin change interrupt
  GIMSK = 0b00100000;                           // turn on pin change interrupts
  PCMSK = bit(BUTTON);                          // turn on interrupt on button pin

  // disable ADC for energy saving
  ADCSRA = 0;                                   // disable ADC
}


void loop() {
  updateCandle();                               // candle simulation

  if (!bitRead(PINB, BUTTON)) {                 // if button is pressed
    DDRB = bit(MOSFET);                         // LED pins as input (PWM off)
    bitClear(PORTB, MOSFET);                    // switch off MOSFET
    delay(10);                                  // debounce button
    while (!bitRead(PINB, BUTTON));             // wait for button released
    delay(10);                                  // debounce button
    sleep();                                    // sleep until button pressed
    DDRB = bit(LED0) | bit(LED1) | bit(MOSFET); // LED pins as output (PWM on)
    bitSet(PORTB, MOSFET);                      // switch on MOSFET
    delay(10);                                  // debounce button
    while (!bitRead(PINB, BUTTON));             // wait for button released
  }
  
  delay(CANDLEDELAY);                           // delay
}


// Pseudo random number generator
uint16_t prng(uint32_t maxvalue) {
  rn = (rn >> 0x01U) ^ (-(rn & 0x01U) & 0xB400U);
  return( (maxvalue * rn) >> 16 );
}


// Candle simulation
void updateCandle() {
  int movx=0;
  int movy=0;
   
  // random poke, intensity determined by uncalm value (0 is perfectly calm)
  movx = prng(uncalm>>8) - (uncalm>>9);
  movy = prng(uncalm>>8) - (uncalm>>9);
  
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
    xvel = (xvel *999)/1000;
    yvel = (yvel *999)/1000;
  }

  // apply acceleration towards center, proportional to distance from center (spring motion; hooke's law)
  xvel -= centerx;
  yvel -= centery;

  // set LEDs
  OCR0A = 155 + centerx;
  OCR0B = 155 + centery;
}


// go to sleep in order to save energy, wake up again by pin change interrupt (button pressed)
void sleep() {
  set_sleep_mode (SLEEP_MODE_PWR_DOWN); // set sleep mode to power down
  bitSet (GIFR, PCIF);                  // clear any outstanding interrupts
  power_all_disable ();                 // power everything off
  noInterrupts ();                      // timed sequence coming up
  sleep_enable ();                      // ready to sleep
  interrupts ();                        // interrupts are required now
  sleep_cpu ();                         // sleep              
  sleep_disable ();                     // precaution
  power_all_enable ();                  // power everything back on
}


// pin change interrupt service routine
EMPTY_INTERRUPT (PCINT0_vect);          // nothing to be done here, just wake up from sleep
