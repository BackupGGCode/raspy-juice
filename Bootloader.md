# 1.0 Introduction #

**Raspy Juice** is an experimental expansion board with an AVR ATmega168A microcontroller (MCU) to provide the services of controlling 4 channels of RC servo outputs, an RS485 interface, and a half-duplex software-based RS232 interface. The MCU itself is interfaced to the host computer through an I2C/TWI interface, in which the former is a slave device.

The MCU itself has to programmed (flashed) with a firmware application so that the above services are operational. For the purpose of developing this firmware, the initial method of flashing the MCU is by using an _AVR Programmer_ with a 6-pin ICSP (in-circuit serial programming) header. However, this is inconvenient for users who do not own such a programming adapter. Also, it is foreseen that future updates of the firmware will need a means of being flashed into the MCU without the use of a programming adapter.

A **_bootloader_** is a tiny program that runs on the MCU when it is powered-up, before the main application takes over. The role of the bootloader is to check for certain conditions and decide if the main application firmware should be updated. If it is to be updated, the bootloader will wait for commands/data from the host computer and flash this new information into the MCU. This bootloader can be developed to use the I2C/TWI interface to the host computer to receive its commands/data.

This draft document is to plan for the development of this bootloader if necessary.

---

# 2.0 Details #

## 2.1 Summary of quick searches of AVR I2C/TWI bootloaders ##
  * [Brad Schick's AVR Bootloader FAQ](http://blog.schicks.net/wp-content/uploads/2009/09/bootloader_faq.pdf)
  * [Atmel AVR112: TWI Bootloader for devices without boot section](http://www.atmel.com/Images/doc8079.pdf)
  * [Atmel AVR1327: Two Wire Interface (TWI) Slave Bootloader for Atmel AVR XMEGA](http://www.atmel.com/Images/doc8435.pdf)
  * [Atmel AVR1622: TWI Boot Loader for XMEGA](http://www.atmel.com/Images/doc8437.pdf)
  * [A bootloader for the ATmega8 AVR microprocessor](http://www.dl5neg.de/bootloader/bootloader.html)
  * [Robostix i2c bootloader](http://docwiki.gumstix.com/Robostix_i2c_bootloader)
  * [OpenServo Bootloader sources](http://www.openservo.com/viewcvs/OpenServo/ATmegaX_Bootloader/?root=cvs)
  * [Open-source twiboot I2C/TWI  bootloader for AVR](http://git.kopf-tisch.de/?p=twiboot)
  * [avr-xboot Google Code project](http://code.google.com/p/avr-xboot/)
  * [Testing AVR universal bootloader on Atmega128](http://www.scienceprog.com/testing-avr-universal-bootloader-on-atmega128/)

The open-sourced **_"twiboot I2C/TWI  bootloader for AVR"_** listed above compiles with AVR Studio 4 and avr-gcc (WinAVR) almost out of the box. The source also comes with a small linux application that programs the MCU through the I2C/TWI interface. With small patching for MCU type and LED pin re-definition, twiboot has been adopted for the Raspy Juice I2C/TWI bootloading.

## 2.2 Basic Description of _twiboot_ ##
On power-up or on reset, twiboot will flash the LED rapidly awaiting commands from the host computer for 1 second. During this period, the bootloader has an I2C slave address of 0x29.

```
root@bg3:~# i2cdetect -r -y 2
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- 29 -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
root@bg3:~# root
```
In the above log of Juice tested against a beagleboard sitting on the I2C bus #2, **0x68** shows the presence of the former's PCF8523 real-time clock chip, and **0x29** shows the AVR in bootloader mode.
If commanded during this one-second period, the bootloader will wait for further instructions from the host computer, else it will attempt to start the main firmware application already present in the MCU.

```
root@bg3:~# i2cdetect -r -y 2
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
root@bg3:~#
```
In the above log, the bootloader has timed-out or exited, and started the main application firmware. This firmware has now adopted an I2C address of **0x48**.

## 2.3 Reprogramming/flashing Juice with twiboot ##
Flashing the Juice AVR chip is by using the command:
```
root@bg3:~# twiboot -a 0x29 -d /dev/i2c-2 -w flash:juice.hex
```
However, the AVR chip must be forced into the bootloader mode before the above command can work. There are two ways to achieve this:

#### The hardware way ####
Reset the chip by shorting the ICSP header pins of reset and ground. These are pins 5 and 6 on the header. One would have a one-second period to type the above command line to flash the MCU.

#### The software way ####
The easier way is that if an application firmware already existed on the MCU and has provisions for recognising an I2C command to go into reset/bootloader mode. As such, it should be almost mandatory to write the main application firmware to recognise such a reset command. Juice main application firmware is being developed to recognise such an I2C command. Then, re-flashing the firmware will look like the below command line and log:
```
root@bg3:~# i2cset -y 2 0x48 0xb0 0x0d ; twiboot -a 0x29 -d /dev/i2c-2 -w flash:juice.hex
device         : /dev/i2c-2       (address: 0x29)
version        : TWIBOOT m168v2.1”
                                  €< (sig: 0x1e 0x94 0x0b => unknown)
flash size     : 0x3c00 / 15360   (0x80 bytes/page)
eeprom size    : 0x0200 /   512
writing flash  : [**************************************************á] (4716)
verifing flash : [**************************************************] (4716)
root@bg3:~#
```


---

# 3.0 Testing Raspy Juice on Raspberry Pi with Debian Wheezy #

Testing Raspy Juice with Raspberry Pi stock Debian image of Wheezy. I2C modules are blacklisted by default in /etc/modprobe.d/raspi-blacklist.conf
```
Reason of
# blacklist spi and i2c by default (many users don't need them)
```
It's OK because modules can be preloaded at linux boot time in /etc/modules.conf or /etc/modules

Add to /etc/modules (_you'd have to be root to do this, or sudo_) and reboot.
```
i2c-dev
i2c-bcm2708
```

After which
```
pi@rpdev /etc $ ls -l /dev/*i2c*
crw-rw---T 1 root root 89, 0 Aug  4 04:19 /dev/i2c-0
crw-rw---T 1 root root 89, 1 Aug  4 04:19 /dev/i2c-1
```

## 3.1 Interaction with I2C Busses and Devices ##
For interacting with I2C busses and devices, the tools will have to be installed. Also, non-root users may need to be upgraded to interact with I2C devices.
```
sudo apt-get update
sudo apt-get install i2c-tools
sudo usermod -aG i2c yourusername
```

Testing the above tools installation
```
pi@rpdev /etc $ /usr/sbin/i2cdetect -l
i2c-0    i2c           bcm2708_i2c.0                       I2C adapter
i2c-1    i2c           bcm2708_i2c.1                       I2C adapter

pi@rpdev ~ $ /usr/sbin/i2cdetect -y 0
0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- 68 -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

## 3.2 Building the Firmware Uploader ##
Checkout source code from this site
```
sudo apt-get update
sudo apt-get install subversion
cd
svn checkout http://raspy-juice.googlecode.com/svn/trunk/ raspy-juice-read-only
```

Build the AVR _twiboot_ I2C/TWI uploader
```
cd ~/raspy-juice-read-only/bootloader/razzor-twiboot-ca2a0a9/linux/
make
sudo cp twiboot /usr/local/bin
```
[**_It is found that if libi2c-dev was installed prior to the above command, the application will not build properly. So remove it before performing the above step._**]

## 3.3 Updating the Firmware ##
Install tools for linux AVR code development
```
sudo apt-get update
sudo apt-get install gcc-avr binutils-avr avr-libc libi2c-dev
```

Build the AVR firmware service application.
```
cd ~/raspy-juice-read-only/firmware
make
```

Upload and flash the AVR microcontroller
```
/usr/sbin/i2cset -y 0 0x48 0xb0 0x0d  ; sleep 0.5 ; twiboot -a 0x29 -w flash:juice.hex
make flashinstall

```
[**_The above assumes that the AVR has a firmware application that recognises the reboot command of '0xb0 0x0d'. If the AVR-PD7 LED on Raspy Juice did not flash rapidly during the above step, then the hardware way is required to reboot the AVR microcontroller prior to completing the above command: a. Type the above command but don't hit enter key, b. Briefly short pins 5 and 6 on the AVR-ISP header (yikes) but the LED will start to flash rapidly, c. Press the  enter key._**]

## 3.4 Quickie Testing with an RC Servo ##
Plug in an RC servo into Servo 1 port.
```
/usr/sbin/i2cset -y 0 0x48 1 1000 w
/usr/sbin/i2cset -y 0 0x48 1 2000 w
while [ 1 ] ; do /usr/sbin/i2cset -y 0 0x48 1 1000 w ; sleep 0.5 ; /usr/sbin/i2cset -y 0 0x48 1 2000 w ; sleep 0.5 ; done
^C
```



---
