#Userspace example of interacting with Raspy Juice.

# 1. Introduction **DRAFT DRAFT DRAFT -- DO NOT RELY YET :-(** #

Add your content here.



Raspy Juice (**_Juice_**) is primarily a power supply board for the Raspberry Pi (**_RPi_**) supplying a regulated +5V via the GPIO header, and with added conveniences of a real-time clock (RTC) and an RS232-level console port. However, it also sports an Atmel ATmega168A AVR microcontroller (**_MCU_**) attached to the RPi as an I2C slave, and has its GPIO and analog pins exposed out for interfacing experiments. See HardwareDescription. The MCU itself is running a firmware service that emulates an I2C register set, which allows a userspace application to control and interact with the interfaces.

This document is to show how to interact with this AVR microcontroller from user-written application programs.

# 2. Prerequisites #

For a userspace application to interact with the AVR MCU interfaces, the are several prerequisites that must be met.
  1. the linux kernel must be enabled with the I2c bus driver
  1. the Raspy Juice MCU must be seen as an I2C slave on the I2C bus
  1. communicate with the Raspy Juice firmware services through its emulated I2C registers, to control the interfaces.

### 2.1 Setting up the linux kernel I2C bus ###
Generally everything written here is for the RPi running the Raspbian/Debian/Wheezy/armhf stock distribution from the Raspberry Pi foundation.
In this distribution, the I2C modules are blacklisted by default in /etc/modprobe.d/raspi-blacklist.conf. It is OK to leave this file untouched, because modules can be instructed to pre-load at linux boot time by the /etc/modules.conf or **/etc/modules** files.

Edit **/etc/modules** and add the two lines so that it will at least look like the two i2c lines are there. (you'd have to be root to do this, or sudo) and reboot.
```
# /etc/modules: kernel modules to load at boot time.
#
# This file contains the names of kernel modules that should be loaded
# at boot time, one per line. Lines beginning with "#" are ignored.
# Parameters can be specified after the module name.

snd-bcm2835
i2c-dev
i2c-bcm2708
```
Then reboot. After which, you should be able to list the i2c busses in the **/dev** directory loike below:
```
pi@rpdev /etc $ ls -l /dev/*i2c*
crw-rw---T 1 root root 89, 0 Aug  4 04:19 /dev/i2c-0
crw-rw---T 1 root root 89, 1 Aug  4 04:19 /dev/i2c-1
```

### 2.2 Verifying Raspy Juice as an I2C device on the kernel bus ###
For interacting with I2C busses and devices, the proper tools will have to be installed.

```
sudo apt-get update
sudo apt-get install i2c-tools
```
When the above tools are installed, a group called **_i2c_** is automatically created. When the above listing of /dev is performed again, the I2C busses will be co-owned by this group.
```
pi@rpdev /etc $ ls -l /dev/*i2c*
crw-rw---T 1 root i2c 89, 0 Jan  1  1970 /dev/i2c-0
crw-rw---T 1 root i2c 89, 1 Jan  1  1970 /dev/i2c-1
```



Now, non-root users need to be upgraded to interact with I2C busses and devices.
```
sudo usermod -aG i2c yourusername
```


Then, verify that Raspy Juice is on the I2C-0 bus.
```
pi@rpdev ~ $ ls -l /dev/i2c*
crw-rw---T 1 root i2c 89, 0 Jan  1  1970 /dev/i2c-0
crw-rw---T 1 root i2c 89, 1 Jan  1  1970 /dev/i2c-1

pi@rpdev ~ $ i2cdetect -l
i2c-0	i2c       	bcm2708_i2c.0                   	I2C adapter
i2c-1	i2c       	bcm2708_i2c.1                   	I2C adapter

pi@rpdev ~ $ i2cdetect -y 0
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- UU -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
pi@rpdev ~ $ 
```
In the above log, _i2cdetect_ has detected two devices sitting on I2C bus 0 or /dev/i2c-0. **_'UU'_** is a device that has been registered with the kernel and generally unwise to interact with such a kernel-owned device. In this case, the Raspy Juice NXP8523 RTC has been registered with the kernel with an I2C slave address of 0x68. **_'48'_** is the Raspy Juice MCU with a firmware service emulating as an I2C slave device, sitting on the bus 0 with an address of **0x48**.


# 3. Communicating with Raspy Juice #

Reading / writing to Raspy Juice I2C registers. See I2CemulatedSlaveRegisters. Need i2c-dev and libraries.

```
sudo apt-get update
sudo apt-get install libi2c-dev
```

### 3.1 Opening a I2C bus device ###
The below example is a snippet to open the Raspy Juice as an I2C slave device and exit.
```
#include <linux/i2c-dev.h>
#include <linux/fcntl.h>
#include "../firmware/juice.h"

int main(int argc, char *argv[])
{
    char devbusname[] = "/dev/i2c-0";
    int i2caddr = AVRSLAVE_ADDR;

    file = open(devbusname, O_RDWR);
    if (file < 0) {
        printf("open %s: error = %d\n", devbusname, file);
        exit(1);
    }
    else
        printf("open %s: succeeded.\n", devbusname);

    if (ioctl(file, I2C_SLAVE, i2caddr) < 0) {
        printf("open i2c slave 0x%02x: error = %s\n\n", i2caddr, "dunno");
        exit(1);
    }
    else
        printf("open i2c slave 0x%02x: succeeded.\n\n", i2caddr);

    return 0;
}
```
The first two include header files are necessary for opening the I2C bus and device as a file, and the third include header file are the Raspy Juice constants.
### 3.2 Reading and Writing to a Juice I2C register ###
To perform read and writes to the Raspy Juice emulated registers, the i2c-dev library calls of accessing I2C device registers are used.
Using the libi2c library call for read byte,
```
        rval = i2c_smbus_read_byte_data(file, subreg); 
```

Write byte
```
        rval = i2c_smbus_write_byte_data(file, subreg, data);
```
Read word, or double-byte
```
        rval = i2c_smbus_write_byte_data(file, subreg, data);
```
Write word, or double-byte
```
        rval = i2c_smbus_write_word_data(file, subreg, data);
```
Sometimes, a call to one of the above may fail due to unforseen reasons, so the programmer may implement a retry mechanism for slightly more robust read/writes.

### 3.3 Controlling the servo outputs ###
The servo I2C registers taken a double-byte parameter specifying the PWM assertion period of 0.5ms to 2.5ms or, 500usec to 2500usec. So, for Juice servo register, this parameter is written as two bytes, the first byte being the low-significant byte, followed by the most-significant byte in usec (microseconds).

```
int rj_setservo(int chan, int usec)
{
    return i2c_smbus_write_word_data(file, chan, usec);
}
```



### 3.4 Reading the firmware version ###

### 3.5 Reading the status register ###

### 3.6 Reading the analog-to-digital convertors ###

### 3.7 RS232 Interface ###

### 3.8 RS485 Interface ###

### 3.9 GPIO Interface ###