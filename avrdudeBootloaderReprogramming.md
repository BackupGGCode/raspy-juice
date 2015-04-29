# Introduction (DRAFT) #
This page describes the re-programming of the bootloader firmware into the AVR ATmega168 microcontroller using AVR Serial ISP (In-System Programming) technique with the Raspberry Pi GPIO pins.

# 2.0 Details (DRAFT) #
## 2.1 Summary of quick searches of Raspberry Pi AVR Serial Programming ##

  * http://www.raspberrypi.org/phpBB3/viewtopic.php?f=42&t=22650
  * http://blog.stevemarple.co.uk/2012/07/avrarduino-isp-programmer-using.html
  * http://blog.stevemarple.co.uk/2013/03/how-to-use-gpio-version-of-avrdude-on.html
  * http://www.repyoblog.com/index.php/2012/10/raspberry-pi-with-avr-328p-using-spi-and-gpio/
  * http://linux.die.net/man/1/avrdude
  * http://www.nongnu.org/avrdude/
  * http://www.engbedded.com/fusecalc/
  * https://projects.drogon.net/raspberry-pi/gertboard/arduino-ide-installation-isp/
  * https://aur.archlinux.org/packages/avrdude-svn/

### 2.x.x Installing, Pros and Cons ###
  * Debian apt-get: Version 5.10 -- no linuxgpio support.
  * Debian patched: Version 5.10 -- from https://projects.drogon.net/raspberry-pi/gertboard/arduino-ide-installation-isp/ -- inverted Reset pin setting. Try not to use pins of Raspberry Pi SPI because after avrdude usage, it will crap out the spi\_bcm2708 module (and have to rmmod & modprobe again). Only works root mode, can set class-gpio pins to non-root permissions, but will revert to root ownership after avrdude usage.
  * ArchLinuxARM pacman: Verson 5.11 -- distributed version has no linuxgpio support. Looks like have to pkgbuild from https://aur.archlinux.org/packages/avrdude-svn/

### 2.x.x Basic command line instructions to flash the AVR ###

[Fixme: explain Fuse settings, esp bootrst & bootsz]<br>
[Fixme: EEPROM programming]<br>
<pre><code># avrdude -c linuxgpio -p m168<br>
<br>
# avrdude -c linuxgpio -p m168 -U lfuse:w:0xE6:m<br>
<br>
# avrdude -c linuxgpio -p m168 -U hfuse:w:0xD7:m<br>
<br>
# avrdude -c linuxgpio -p m168 -U efuse:w:0x02:m<br>
<br>
# avrdude -c linuxgpio -p m168 -U flash:w:twiboot-win.hex<br>
<br>
# # modify eep.raw file into updated eep-new.raw, then write back<br>
# avrdude -c linuxgpio -p m168 -U eeprom:r:eep.raw:r<br>
# avrdude -c linuxgpio -p m168 -U eeprom:w:eep-new.raw:r<br>
</code></pre>