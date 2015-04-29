# Introduction #

The sources for the RTC driver is not maintained as a linux source, you will have to build the kernel yourself, using the published patch. Also, in order to use the RTC time at boot, the drivers (I2C and RTC) need to be compiled into the kernel, I doubt that this will ever be done in standard distributions.

I got this recepie from Adnan at 2-Watt elements. I have sucessfully compiled a new kernel on an Arch i686, se below for modified instructions.

# Details #

Preparation of my build environment (Ubuntu).
```
$ sudo apt-get install build-essential git patch libncurses5-dev
$ sudo apt-get install libqt4-dev # optional for make xconfig #
```
Download the cross-compiler and copy to /usr/local/ (I used the linaro cross-compiler from Raspberry Pi github).
```
$ cd
$ git clone https://github.com/raspberrypi/tools.git rpi-tools.git
$ sudo cp -rpv rpi-tools.git/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/* /usr/local
```
Download the Raspberry Pi linux sources.
```
$ cd
$ git clone git://github.com/raspberrypi/linux.git rpi-linux.git
```
Apply the Raspy Juice patches for the RTC module.
```
$ cd rpi-linux.git
$ patch -p1 < ../raspy-juice-read-only/linux-rtc/0001-rtc-pcf8523.patch
$ patch -p1 < ../raspy-juice-read-only/linux-rtc/0002-pcf8523-i2c-register-dt.patch
```
Apply the configuration. Here, I ensure the drivers for the Raspberry Pi I2C modules are selected. Then select the NXP PCF8523 driver under the section of Real Time Clock. Save the config and exit.
```
$ cp arch/arm/configs/bcmrpi_defconfig .config
$ make ARCH=arm CROSS_COMPILE=/usr/local/bin/arm-linux-gnueabihf- xconfig
```
Compile the kernel and modules.
```
$ make ARCH=arm CROSS_COMPILE=/usr/local/bin/arm-linux-gnueabihf- INSTALL_MOD_PATH=arch/arm/boot/ zImage modules modules_install
```
Mounted the SDcard onto the Ubuntu desktop, and copy the image and modules over to the SDcard.
```
$ sudo mkdir -p /mnt/sdX1 /mnt/sdX2
$ sudo mount /dev/sdX1 /mnt/sdX1
$ sudo mount /dev/sdX2 /mnt/sdX2

$ cd arch/arm/boot
$ sudo cp /mnt/sdb1/kernel.img kernel-rpi.img # make a backup #
$ sudo cp zImage /mnt/sdX1/kernel.img
$ sudo cp -rpv lib /mnt/sdb2
$ sudo umount /mnt/sdX1 /mnt/sdX2
```
Removed the SDcard from the Ubuntu desktop, plug it in and power up the Raspberry Pi.

# Compiling on Arch i686 #

I have sucessfully compiled the kernel on my Arch i686 (32 bit) development box. It is basically the same as for Ubuntu, except for pacman being used instead of "apt-get install". I had to modify how to copy the lib sligthly. I also have automount on my dev box so there are no mount commands.

```
# pacman -S base-devel git qt svn
$ svn checkout http://raspy-juice.googlecode.com/svn/trunk/raspy-juice-read-only
$ git clone https://github.com/raspberrypi/tools.git rpi-tools.git
$ sudo cp -rpv rpi-tools.git/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/* /usr/local
$ git clone git://github.com/raspberrypi/linux.git rpi-linux.git
$ cd rpi-linux.git/
$ patch -p1 < ../raspy-juice-read-only/linux-rtc/0001-rtc-pcf8523.patch
$ cp arch/arm/configs/bcmrpi_defconfig .config
$ make ARCH=arm CROSS_COMPILE=/usr/local/bin/arm-linux-gnueabihf- xconfig
```

Configure both I2C driver and RTC driver to be kernel (not module) drivers:
```
Symbol: I2C_BCM2708 [=y]
Symbol: RTC_DRV_PCF8523 [=y]
```

```
$ make ARCH=arm CROSS_COMPILE=/usr/local/bin/arm-linux-gnueabihf- INSTALL_MOD_PATH=arch/arm/boot/ zImage modules modules_install
$ cd arch/arm/boot/
$ sudo cp /media/hd-sdc1/kernel.img kernel-rpi.img
$ sudo cp zImage /media/hd-sdc1/kernel.img
$ sudo cp -rpv lib/* /media/hd-sdc2/lib
$ sudo chown -R root:root /media/hd-sdc2/lib
$ sudo umount /media/hd-sdc1
$ sudo umount /media/hd-sdc2
```

On Raspberry after boot, check that the driver is loaded:

```
# cd /var/log
# grep pcf8523 kernel.log
Oct 30 14:48:03 alarmpi kernel: [    1.187464] rtc-pcf8523 0-0068: chip found, driver version 1.0
Oct 30 14:48:03 alarmpi kernel: [    1.189455] rtc-pcf8523 0-0068: rtc core: registered rtc-pcf8523 as rtc0
Oct 30 14:48:03 alarmpi kernel: [    1.197542] rtc-pcf8523 0-0068: setting system clock to 2012-10-30 13:47:57 UTC (1351604877)
```

In order to set the timezone I had to install polkit and reboot:

```
# pacman -S polkit
```

Setting the clock:

```
# timedatectl set-timezone Europe/Stockholm
# /sbin/hwclock --systohc --utc
```

Reboot.

Check that all log files have correct date and that system is started with correct time:

```
# cd /var/log
# grep pcf8523 kernel.log
# ls -l *
```