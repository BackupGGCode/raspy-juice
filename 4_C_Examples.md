# 1. Introduction **DRAFT IN PROGRESS** #

---

Raspy Juice (**_Juice_**) is primarily a power supply board for the Raspberry Pi (**_RPi_**) supplying a regulated +5V via the GPIO header, and with added conveniences of a real-time clock (RTC) and an RS232-level console port. However, it also sports an Atmel ATmega168A AVR microcontroller (**_MCU_**) attached to the RPi as an I2C slave, and has its GPIO and analog pins exposed out for interfacing experiments. See HardwareDescription. The MCU itself is running a firmware service that emulates an I2C register set, which allows a userspace application to control and interact with the interfaces.

This document is to show how to interact with this AVR microcontroller from user-written application programs.
<br>
<br>
<br>

<h1>2. Prerequisites</h1>
<hr />
For a userspace application to interact with the AVR MCU interfaces, the are several prerequisites that must be met.<br>
<ol><li>the linux kernel must be enabled with the I2c bus driver (see <a href='http://code.google.com/p/raspy-juice/wiki/3_Updating_the_Firmware#2.1_Setting_up_the_linux_kernel_I2C_bus'>Setting up the linux kernel I2C bus</a> )<br>
</li><li>the Raspy Juice MCU must be seen as an I2C slave on the I2C bus (see <a href='http://code.google.com/p/raspy-juice/wiki/3_Updating_the_Firmware#2.2_Verifying_Raspy_Juice_as_an_I2C_device_on_the_kernel_bus'>http://code.google.com/p/raspy-juice/wiki/3_Updating_the_Firmware#2.2_Verifying_Raspy_Juice_as_an_I2C_device_on_the_kernel_bus</a> Verifying Raspy Juice on I2C...] )<br>
</li><li>communicate and program the Raspy Juice using the lib-juice.c library calls.<br>
<br>
<br>
<br></li></ol>

<h1>3. Compiling with lib-juice.c C library calls</h1>
<hr />


<br>
<br>
<br>

<h1>4. Communicating with Raspy Juice</h1>
<hr />

<h3>4.1 Opening Raspy Juice</h3>
The below example is a snippet to open the Raspy Juice as an I2C slave device and exit.<br>
<pre><code>#include "juice-dev.h"<br>
#include "../firmware/juice.h"<br>
<br>
int main(int argc, char *argv[])<br>
{<br>
    char devbusname[] = "/dev/i2c-0";<br>
    int i2caddr = AVRSLAVE_ADDR;<br>
<br>
    rval = rj_open(devbusname, i2caddr);<br>
    if (rval &lt; 0) {<br>
        printf("open %s: failed, rval = %d\n", devbusname, rval);<br>
        exit(1);<br>
    }<br>
    else<br>
        printf("juice: open at 0x%02x: succeeded.\n", i2caddr);<br>
<br>
    version = rj_getversion();<br>
    if (version != NULL)<br>
        printf("juice: firmware version = %s\n", version);<br>
    else {<br>
        printf("juice: rj_getversion failed.\n");<br>
        exit(2);<br>
    }<br>
    printf("\n");<br>
<br>
    return 0;<br>
}<br>
</code></pre>
The first two include header files are necessary for opening the I2C bus and device as a file, and the third include header file are the Raspy Juice constants.<br>
<br>
<h3>4.2 Reading the firmware version</h3>
<h3>4.3 Controlling the servo outputs</h3>
<h3>4.4 Reading the firmware version</h3>
<h3>4.5 Reading the status register</h3>
<h3>4.6 Reading the analog-to-digital convertors</h3>
<h3>4.7 RS232 Interface</h3>
<h3>4.8 RS485 Interface</h3>
<h3>4.9 GPIO Interface</h3>