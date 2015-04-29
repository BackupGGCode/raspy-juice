# Introduction #

Raspy Juice is a power supply and expansion board for the Raspberry Pi (**RPi**) with a real-time clock (**RTC**) based on an NXP PCF8523 chip. At the time of this writing, the linux kernel 3.6.11 does not contain a device driver for this chip, and the kernel source have to be patched and re-built in order to interact with this chip. [_**The linux upstream release candidate 3.8-rc3 now contains the PCF8523 device driver**_.]

This page describes the various ways to install and use this RTC device driver.

(Some of the details in this page comes from the [ArchArmRTCkernel](ArchArmRTCkernel.md) wiki page).

# Details #

Various ways of using the RTC chip.
1. Build the RTC device driver as a module.
This allows the kernel module to be inserted from user-space. But does not let the kernel set the system time at bootup.
a. compile the device-driver module only, natively on the Raspberry Pi
b. com