| <img src='http://raspy-juice.googlecode.com/svn/wiki/juice-r1-320x200w.jpg' alt='Picture of Raspy Juice - Raspberry Pi combo' width='320' /> | <img src='http://raspy-juice.googlecode.com/svn/wiki/juice-block-640x480.png' width='320' />  |
|:---------------------------------------------------------------------------------------------------------------------------------------------|:----------------------------------------------------------------------------------------------|


# Project #

---

For the **[Raspberry Pi](http://elinux.org/R-Pi_Hub#About)** (RPi) low-cost educational computer, the _**[Raspy Juice Expansion Board](http://code.google.com/p/raspy-juice/wiki/1_Hardware_Description)**_ is a compact piggyback PCB board that power supply the required and regulated +5V to the RPi from a wide range of voltage sources, provides an RS232-level console breakout port and a real-time clock. The secondary feature of _Raspy Juice_ is its expansion ATmega168A microcontroller that has interfaces of an RS485 port, an additional RS232 port, 4x RC servo ports, and an expansion pins header for the MCU. This microcontroller and its interfaces are for simple robotics and other interfacing experiments.

This project page serves as a repository for the open-sources of the expansion microcontroller's _[bootloader/uploader](http://code.google.com/p/raspy-juice/wiki/2_Bootloader)_, _[firmware](http://code.google.com/p/raspy-juice/wiki/3_Updating_the_Firmware)_ and applications.



# News #

---

### 2013-07-03 ###
The Raspy Juice Rev.2 PCBA (Printed Circuit Board) is NOW available. You may click on the Raspy Juice AVAILABILITY link on the left sidebar. (Schematics are available on the [Hardware Description](http://code.google.com/p/raspy-juice/wiki/1_Hardware_Description) page).

### 2013-02-07 ###
The NXP PCF8523 RTC real time clock chip device driver is now patched and supported in the **Arch Linux ARM distribution** for the Raspberry Pi. See this forum thread [Driver for NXP PCF8523 (used by Raspy-Juice) ex 3.8 roadmap?](http://http://archlinuxarm.org/forum/viewtopic.php?f=9&t=4876,) This allows Raspberry Pi-based boxes to
set the system time on boot-up where Internet or NTP services are unavailable. For an Arch Linux ARM installation, a _'pacman -Sf linux-raspberrypi'_ will upgrade the kernel and the PCF8523 RTC device driver module is within.

### 2013-01-18 ###
The Raspy Juice was successfully implemented and used in an industrial application. See this forum thread [Arch Linux Arm (on Raspberry Pi) Success Story](http://archlinuxarm.org/forum/viewtopic.php?f=8&t=4877&p=26873,) by [Vicky Teknik AB](http://www.vickyteknik.se/maxisign/) and [Vicky Teknik AB FB](http://www.facebook.com/VickyTeknik). Their "application is built in python on tornado" (webserver).

### 2013-01-11 ###
A device driver for the NXP PCF8523 real-time clock chip was discovered in the upstream linux-3.8-rc2! Although the current Raspberry Pi kernel-3.6.11 does not contain this driver, it will not be long before linux-3.8 becomes prevalent. This is good news.

This project repository here has adopted that patch http://code.google.com/p/raspy-juice/source/browse/trunk/linux-rtc/0001-rtc-pcf8523.patch as-is for back-porting into the current linux-3.6.11, so that the transition is easier. Also, for setting the system time at kernel boot up (ie. with hctosys.c), the device driver [registration code patch](http://code.google.com/p/raspy-juice/source/browse/trunk/linux-rtc/0002-pcf8523-i2c-register-dt.patch) for board initialization has been fixed to work either Raspberry Pi Rev.1 or Rev.2 boards where the I2C buses were swapped.

#### 2012-12-10 ####
Added the first **Python programming example** of controlling the servos in the source. See https://code.google.com/p/raspy-juice/source/browse/trunk/py_examples/ServoBars.py With python2, Tkinter and python-smbus, this small application controls two RC servos connected to the Raspy Juice.

#### 2012-08-16 ####
The Raspy Juice Rev.1 PCBA Kit is available at [2-Watt Elements at Shopify](http://2-watt-elements.myshopify.com/).
<br>
Also see Raspberry Pi forum sub-section of <a href='http://www.raspberrypi.org/phpBB3/viewtopic.php?f=59&t=14654'>Board index ‹ Ye Olde Pi Shoppe ‹ Add-ons for sale</a>.<br>
<br>
<br>
<hr />
<h5>Raspberry Pi is a trademark of the <a href='http://www.raspberrypi.org/'>Raspberry Pi Foundation</a></h5>