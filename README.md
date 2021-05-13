# TinyCandle - Candle Simulation based on ATtiny13A
A simple tealight candle simulation based on the ATtiny13A.

- Design Files (EasyEDA): https://easyeda.com/wagiminator/attiny13-tinycandle

![TinyCandle_pic3.jpg](https://raw.githubusercontent.com/wagiminator/ATtiny13-TinyCandle/master/documentation/TinyCandle_pic3.jpg)

# Software
## Candle Simulation
The simulation code is based on the great work of [Mark Sherman](https://github.com/carangil/candle). With most LED candles, the random flickering is rather silly and lacks the spooky element of candlelight: shadows cast by the candle move because the flame is in motion. With TinyCandle, four LEDs are arranged in a square, their intensity forms a "center" of light balance that can be moved around smoothly.

This 'physics-based' candlelight has the following properties:
- The flame has a position and a velocity.
- If the position is not centered, the velocity will accelerate towards the center in proportion to the displacement (hooke's law of springs).
- The velocity has a very small damping value, which means that corrections towards the center always overshoot a bit (underdamped system).
- Random "pushes" into the center position of the light are performed to mimic random drafts.
- The strength of the drafts changes periodically (alternating periods of calm and windiness).

## Pseudo Random Number Generator
The implementation of the candle simulation requires random numbers for a realistic flickering of the candle. However, the usual libraries for generating random numbers require a relatively large amount of memory. Fortunately, Łukasz Podkalicki has developed a [lightweight random number generator](https://blog.podkalicki.com/attiny13-pseudo-random-numbers/) based on [Galois linear feedback shift register](https://en.wikipedia.org/wiki/Linear-feedback_shift_register#Galois_LFSRs) for the ATtiny13A, which is also used here, slightly adapted. When compiled, this function only requires **86 bytes of flash**.

```c
// Start state (any nonzero value will work)
uint16_t rn = 0xACE1;

// Pseudo random number generator
uint16_t prng(uint16_t maxvalue) {
  rn = (rn >> 0x01) ^ (-(rn & 0x01) & 0xB400);
  return(rn % maxvalue);
}
```

## Compiling and Uploading Firmware
### If using the Arduino IDE
- Make sure you have installed [MicroCore](https://github.com/MCUdude/MicroCore).
- Go to **Tools -> Board -> MicroCore** and select **ATtiny13**.
- Go to **Tools** and choose the following board options:
  - **Clock:**  1.2 MHz internal osc.
  - **BOD:**    disabled
  - **Timing:** Micros disabled
- Connect your programmer to your PC and to the ICSP header on the board.
- Go to **Tools -> Programmer** and select your ISP programmer (e.g. [USBasp](https://aliexpress.com/wholesale?SearchText=usbasp)).
- Go to **Tools -> Burn Bootloader** to burn the fuses.
- Open the TinyCandle sketch and click **Upload**.

### If using the precompiled hex-file
- Make sure you have installed [avrdude](https://learn.adafruit.com/usbtinyisp/avrdude).
- Connect your programmer to your PC and to the ICSP header on the board.
- Open a terminal.
- Navigate to the folder with the hex-file.
- Execute the following command (if necessary replace "usbasp" with the programmer you use):
  ```
  avrdude -c usbasp -p t13 -U lfuse:w:0x2a:m -U hfuse:w:0xff:m -U flash:w:tinycandle.hex
  ```

### If using the makefile (Linux/Mac)
- Make sure you have installed [avr-gcc toolchain and avrdude](http://maxembedded.com/2015/06/setting-up-avr-gcc-toolchain-on-linux-and-mac-os-x/).
- Connect your programmer to your PC and to the ICSP header on the board.
- Open the makefile and change the programmer if you are not using usbasp.
- Open a terminal.
- Navigate to the folder with the makefile and the Arduino sketch.
- Run "make install" to compile, burn the fuses and upload the firmware.

# References, Links and Notes
1. [ATtiny13A Datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/doc8126.pdf)
2. [Candle Simulation Implementation by Mark Sherman](https://github.com/carangil/candle)
3. [Lightweight Random Number Generator by Łukasz Podkalicki](https://blog.podkalicki.com/attiny13-pseudo-random-numbers/)
4. [NeoCandle based on ATtiny25/45/85](https://github.com/wagiminator/ATtiny85-TinyCandle)

![TinyCandle_pic1.jpg](https://raw.githubusercontent.com/wagiminator/ATtiny13-TinyCandle/master/documentation/TinyCandle_pic1.jpg)
![TinyCandle_pic2.jpg](https://raw.githubusercontent.com/wagiminator/ATtiny13-TinyCandle/master/documentation/TinyCandle_pic2.jpg)

# License
![license.png](https://i.creativecommons.org/l/by-sa/3.0/88x31.png)

This work is licensed under Creative Commons Attribution-ShareAlike 3.0 Unported License. 
(http://creativecommons.org/licenses/by-sa/3.0/)
