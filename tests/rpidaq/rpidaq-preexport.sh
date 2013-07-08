#! /bin/bash

# This script must be run with sudo.
#
# 2013-07-01 adnan@singnet.com.sg
# Adding pre-exporting of ttyAMA0 for use with RPi-DAQ application.
#
# 2013-06-xx adnan@singnet.com.sg
# Raspberry Pi GPIO pre-exporting script for use with avrdude-rpidaq.
# The avrdude-pidaq has been patched so that it doesn't need to be run as
# root for exporting the GPIO pins with sysfs, and also patched that it
# doesn't unexport (close) the pins so that avrdude can run repeatedly.
#
# Just for rpi-daq specific use, the avrdude.conf.in is patched with the
# below-listed GPIO pin usage.

#  id    = "linuxgpio";
#  desc  = "Use sysfs interface to bitbang GPIO lines";
#  type  = "linuxgpio";
#  reset = 17;   # To avoid inconsistency becuz Pi Rev1.GPIO21 -> Rev2.GPIO27 
#  sck   = 22;
#  mosi  = 23;
#  miso  = 24;


if [ "$1" == "-x" ] ; then

echo "Unexporting pins"
  for PIN in 17 22 23 24
  do
    echo "$PIN" > /sys/class/gpio/unexport
  done
  
else

  echo "Exporting pins"
  for PIN in 17 22 23 24
  do
    echo "$PIN" > /sys/class/gpio/export
    /usr/bin/chown -RHh :users /sys/class/gpio/gpio"$PIN"
    /usr/bin/chmod -R og+rw /sys/class/gpio/gpio"$PIN"
  done

  echo "out" > /sys/class/gpio/gpio17/direction
  echo "out" > /sys/class/gpio/gpio22/direction
  echo "out" > /sys/class/gpio/gpio23/direction
  echo "in" >  /sys/class/gpio/gpio24/direction

  echo "1" > /sys/class/gpio/gpio17/value
  echo "0" > /sys/class/gpio/gpio22/value
  echo "0" > /sys/class/gpio/gpio23/value
fi

echo "Directory of /sys/class/gpio:"
/usr/bin/ls -l --color=auto /sys/class/gpio

chown root:tty /dev/ttyAMA0
chmod g+rw /dev/ttyAMA0

echo "Directory of relevant devices in /dev"
ls -l  --color=auto /dev/i2c* /dev/spi* /dev/ttyA* /dev/rtc*
