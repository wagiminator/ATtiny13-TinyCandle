# TinyCandle - Candle Simulation based on ATtiny13A
A simple tealight candle simulation based on the ATtiny13A.

- Design Files (EasyEDA): https://easyeda.com/wagiminator/attiny13-tinycandle

![TinyCandle_pic1.jpg](https://raw.githubusercontent.com/wagiminator/ATtiny13-TinyCandle/master/documentation/TinyCandle_pic1.jpg)

# Compiling and Uploading Firmware
## If using the Arduino IDE
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

## If using the precompiled hex-file
- Make sure you have installed [avrdude](https://learn.adafruit.com/usbtinyisp/avrdude).
- Connect your programmer to your PC and to the ICSP header on the board.
- Open a terminal.
- Navigate to the folder with the hex-file.
- Execute the following command (if necessary replace "usbasp" with the programmer you use):
  ```
  avrdude -c usbasp -p t13 -U lfuse:w:0x2a:m -U hfuse:w:0xff:m -U flash:w:tinycandle.hex
  ```

## If using the makefile (Linux/Mac)
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

![TinyCandle_pic2.jpg](https://raw.githubusercontent.com/wagiminator/ATtiny13-TinyCandle/master/documentation/TinyCandle_pic2.jpg)

# License
![license.png](https://i.creativecommons.org/l/by-sa/3.0/88x31.png)

This work is licensed under Creative Commons Attribution-ShareAlike 3.0 Unported License. 
(http://creativecommons.org/licenses/by-sa/3.0/)
