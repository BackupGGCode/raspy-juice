# Introduction #

Old News.


# Details #

#### 2012-11-23 ####
The Raspy Juice service firmware [Revision 156](https://code.google.com/p/raspy-juice/source/detail?r=156) is **_discovered broken_**: The SDA signal line of I2C interface is pulled low on power-up, preventing the PCF8523 and the AVR microcontroller from operating properly. Please revert to service firmware [Revision 134](https://code.google.com/p/raspy-juice/source/detail?r=134) for the time being. You may checkout the source tree with the command:
```
svn checkout -r 134 http://raspy-juice.googlecode.com/svn/trunk/ raspy-juice.134
```
and follow the instructions on [Updating the Firmware](http://code.google.com/p/raspy-juice/wiki/3_Updating_the_Firmware).
I am still investigating the root cause of this.

#### 2012-11-08 ####
There are some Raspy Juice units out there with the PCF8523 Real Time Clock control register bits incorrectly set -- **_the timekeeping functions will not survive a power recycle!_** The updated service firmware corrects this by setting the appropriate bits on power-up. Please update the firmware to [Revision 156](https://code.google.com/p/raspy-juice/source/detail?r=156) or later. Thanks to Mårten Gustafsson for making observations on his units that led to this discovery. Also thanks to Mårten for kick-starting a wiki page that enlightens the use of Arch Linux ARM for firmware compilation & flashing, and kernel re-building for the RTC driver.

#### 2012-09-28 ####
The AVR [service firmware](http://code.google.com/p/raspy-juice/source/browse/) is slightly updated with the AVR-RS232 and AVR-RS485 interfaces having reconfigurable baud rates through the [C library](http://code.google.com/p/raspy-juice/source/browse/trunk/c_example/lib-juice.c) calls: 1200 to 38400bps for the RS232 interface, and 1200 to 230400bps for the RS485.

#### 2012-09-10 ####
After some struggle, the Juice firmware's I2C/TWI interrupt service routine was revamped
to follow Atmel's AVR311 application note closely, because the AVR TWI module is extremely timing sensitive in the I2C slave data transmission mode. This caused some data loss in the double-byte read-back of Juice's emulated I2C register set. Finally, the **_analog-to-digital convertor (ADC)_** starts to work.

#### 2012-08-24 ####
Started basic framework of Juice firmware (mainly copied code from _tests_ over to _firmware_ directory) http://code.google.com/p/raspy-juice/source/browse/#svn%2Ftrunk%2Ffirmware. At least the blinking LED, the servo, rs485 and rs232 interfaces work.

#### 2012-08-21 ####
Found, fixed and uploaded into the repository: the [NXP PCF8523 RTC linux device driver](https://code.google.com/p/raspy-juice/source/browse/trunk/linux-rtc/).
#### 2012-08-17 ####
This frontpage is undergoing some changes.