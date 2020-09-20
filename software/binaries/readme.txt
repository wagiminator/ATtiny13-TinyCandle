avrdude -c usbtiny -p t13 -U flash:w:tinycandle.hex
avrdude -c usbtiny -p t13 -U hfuse:w:0xfb:m -U lfuse:w:0x62:m
