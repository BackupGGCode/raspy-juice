# 1. Introduction #

---

Raspy Juice (**_Juice_**) is primarily a power supply board for the Raspberry Pi (**_RPi_**) supplying a regulated +5V via the GPIO header, and with added conveniences of a real-time clock (RTC) and an RS232-level console port. However, it also sports an Atmel ATmega168A AVR microcontroller (**_MCU_**) attached to the RPi as an I2C slave, and has its GPIO and analog pins exposed out for interfacing experiments. See HardwareDescription. The MCU itself is running a firmware service that emulates an I2C register set, which allows a userspace application to control and interact with the interfaces.

This document shows how to update the firmware program of this AVR microcontroller.



<br>
<br>
<br>

<h1>2. Prerequisites</h1>
<hr />
For a user to interact with, and update the MCU firmware, there are several prerequisites that must be met.<br>
<ol><li>the linux kernel must be enabled with the I2C bus driver<br>
</li><li>the MCU must be seen as an I2C slave on the linux I2C bus<br>
</li><li>the MCU must be able to be commanded into <i>bootloader</i> mode<br>
</li><li>the <i>bootloader</i> tools must be installed or built<br>
</li><li>the firmware must be available or built</li></ol>

<h2>2.1 Setting up the linux kernel I2C bus</h2>
Generally everything written here is for the RPi running the Raspbian/Debian/Wheezy/armhf stock distribution from the Raspberry Pi foundation.<br>
In this distribution, the I2C modules are blacklisted by default in /etc/modprobe.d/raspi-blacklist.conf. It is OK to leave this file untouched, because modules can be instructed to pre-load at linux boot time by the /etc/modules.conf or <b>/etc/modules</b> files.<br>
<br>
Edit <b>/etc/modules</b> and add the two lines so that it will at least look like the two i2c lines are there. (you'd have to be root to do this, or sudo) and reboot.<br>
<pre><code># /etc/modules: kernel modules to load at boot time.<br>
#<br>
# This file contains the names of kernel modules that should be loaded<br>
# at boot time, one per line. Lines beginning with "#" are ignored.<br>
# Parameters can be specified after the module name.<br>
<br>
snd-bcm2835<br>
i2c-dev<br>
i2c-bcm2708<br>
</code></pre>
Then reboot. After which, you should be able to list the i2c busses in the <b>/dev</b> directory like below:<br>
<pre><code>pi@raspberry ~ $ ls -l /dev/*i2c*<br>
crw-rw---T 1 root root 89, 0 Aug  4 04:19 /dev/i2c-0<br>
crw-rw---T 1 root root 89, 1 Aug  4 04:19 /dev/i2c-1<br>
</code></pre>

<h2>2.2 Verifying Raspy Juice as an I2C device on the kernel bus</h2>
For interacting with I2C busses and devices, the proper tools will have to be installed.<br>
<br>
<pre><code>sudo apt-get update<br>
sudo apt-get install i2c-tools<br>
</code></pre>
When the above tools are installed, a group called <b><i>i2c</i></b> is automatically created. When the above listing of /dev is performed again, the I2C busses will be co-owned by this group.<br>
<pre><code>pi@raspberry /etc $ ls -l /dev/*i2c*<br>
crw-rw---T 1 root i2c 89, 0 Jan  1  1970 /dev/i2c-0<br>
crw-rw---T 1 root i2c 89, 1 Jan  1  1970 /dev/i2c-1<br>
</code></pre>

Now, non-root users need to be upgraded to interact with I2C busses and devices.<br>
<pre><code>sudo usermod -aG i2c yourusername<br>
</code></pre>


Then, verify that Raspy Juice is on the I2C-0 bus.<br>
<pre><code>pi@raspberry ~ $ i2cdetect -y 0<br>
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f<br>
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- <br>
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- <br>
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- <br>
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- <br>
40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- -- <br>
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- <br>
60: -- -- -- -- -- -- -- -- UU -- -- -- -- -- -- -- <br>
70: -- -- -- -- -- -- -- --                         <br>
pi@raspberry ~ $ <br>
</code></pre>
In the above log, <i>i2cdetect</i> has detected two devices sitting on I2C bus 0 or /dev/i2c-0. <b><i>'UU'</i></b> is a device that has been registered with the kernel and generally unwise to interact with such a kernel-owned device. In this case, the Raspy Juice NXP8523 RTC has been registered with the kernel with an I2C slave address of 0x68. <b><i>'48'</i></b> is the Raspy Juice MCU with a firmware service emulating as an I2C slave device, sitting on the bus 0 with an address of <b>0x48</b>.<br>
<br>
<h2>2.3 Verifying the AVR to go into bootloader mode</h2>
There are two ways the AVR MCU can be commanded into bootloader mode. The bootloader mode is visible to the user by the rapidly flashing (<b><i>AVR-PD7</i></b>) LED on the PCBA.<br>
<br>
<h4>The software way</h4>
The easier way is that if the already existing application firmware in the MCU recognises a command-line statement to go into reset/bootloader mode. This can be tested by:<br>
<pre><code>i2cset -y 2 0x48 0xb0 0x0d<br>
</code></pre>
If the AVR-PD7 LED did not flash rapidly for a period of one second, then the previously installed firmware does not recognise the command to go into bootloader mode, or that the firmware has failed. The recovery method is to use the <b><i>hardware way</i></b> described below.<br>
<br>
<br>
<h4>The hardware way</h4>

Reset the chip by shorting the AVR-ICSP header pins of reset and ground <b><i>(yikes)</i></b>. These are pins 5 and 6 on the header. The user will see the AVR-PD7 LED flash rapidly for a one-second period.<br>
<br>
<h2>2.4 Installing the bootloader tools</h2>
Checkout source code from this site<br>
<pre><code>sudo apt-get update<br>
sudo apt-get install subversion<br>
cd<br>
svn checkout http://raspy-juice.googlecode.com/svn/trunk/ raspy-juice-read-only<br>
</code></pre>

Build the AVR <i>twiboot</i> I2C/TWI uploader<br>
<pre><code>cd ~/raspy-juice-read-only/bootloader/razzor-twiboot-ca2a0a9/linux/<br>
make<br>
sudo cp twiboot /usr/local/bin<br>
</code></pre>
[<b><i>It is found that if libi2c-dev was installed prior to the above command, the application will not build properly. So remove it before performing the above step.</i></b>]<br>
<br>
<br>
<br>

<h1>3. Building and Updating the Firmware</h1>
<hr />
Install the tools for linux AVR code development<br>
<pre><code>sudo apt-get update<br>
sudo apt-get install gcc-avr binutils-avr avr-libc libi2c-dev<br>
</code></pre>

<h2>3.1 Build the AVR firmware service application.</h2>
<pre><code>cd ~/raspy-juice-read-only/firmware<br>
make<br>
</code></pre>

Upload and flash the AVR microcontroller<br>
<pre><code>make flashinstall<br>
</code></pre>

[<b><i>The above assumes that the AVR has a firmware application that recognises the reboot command of '0xb0 0x0d'. If the AVR-PD7 LED on Raspy Juice did not flash rapidly during the above step, then the hardware way is required to reboot the AVR microcontroller prior to completing the above command: a. Type the above command but don't hit enter key, b. Briefly short pins 5 and 6 on the AVR-ISP header but the LED will start to flash rapidly, c. Press the  enter key.</i></b>]<br>
<br>
<h2>3.2 Quickie Testing with an RC Servo</h2>
Plug in an RC servo into Servo 1 port.<br>
<pre><code>/usr/sbin/i2cset -y 0 0x48 1 1000 w<br>
/usr/sbin/i2cset -y 0 0x48 1 2000 w<br>
while [ 1 ] ; do /usr/sbin/i2cset -y 0 0x48 1 1000 w ; sleep 0.5 ; /usr/sbin/i2cset -y 0 0x48 1 2000 w ; sleep 0.5 ; done<br>
^C<br>
</code></pre>
<br>

<h1>4. Flashing on Arch Linux ARM</h1>
<hr />
The sections above generally describe the process of building and updating the Raspy Juice firmware on a Raspberry Pi running in a Debian "wheezy" environment. To accomplish the above operations with a Raspberry Pi running Arch Linux ARM, the process is as follows.<br>
<br>
<br>
<h2>4.1 Setting up the Arch Linux ARM kernel I2C bus</h2>
Create or edit a file in the /etc/modules-load.d directory to load the necessary kernel modules at boot-up for the I2C bus. The file contents should at least have:<br>
<pre><code># Load i2c device and bus drivers <br>
i2c-dev<br>
i2c-bcm2708<br>
</code></pre>
The below command may be used to achieve the above (cut and paste).<br>
<pre><code>sudo sh -c 'echo -e "i2c-dev\ni2c-bcm2708\n" &gt;&gt; /etc/modules-load.d/i2c-load.conf'<br>
</code></pre>
<br>

Create, edit or append an additional line to /etc/udev/rules.d/91-local.rules to modify the permissions of I2C devices for user access. The edited file should at least have the line:<br>
<pre><code>SUBSYSTEM=="i2c-dev", MODE="0777", GROUP="i2c"<br>
</code></pre>
The below command may be used to achieve the above (cut and paste).<br>
<pre><code>sudo sh -c 'echo "SUBSYSTEM==\"i2c-dev\", MODE=\"0777\"" &gt;&gt; /etc/udev/rules.d/91-local.rules'<br>
</code></pre>

The owner, group and permissions mode may be adjusted to suit the system needs. After rebooting, the I2C buses should appear in the /dev directory.<br>
<br>
<br>
<h2>4.2 Install the tools to build the bootloader and update the firmware</h2>
<pre><code>sudo pacman -Syy<br>
sudo pacman -S base-devel subversion avr-gcc avr-binutils avr-libc i2c-tools<br>
<br>
cd<br>
svn checkout http://raspy-juice.googlecode.com/svn/trunk/ raspy-juice-read-only<br>
<br>
cd ~/raspy-juice-read-only/bootloader/razzor-twiboot-ca2a0a9/linux/<br>
make<br>
sudo cp twiboot /usr/local/bin<br>
<br>
cd ~/raspy-juice-read-only/firmware<br>
make<br>
make flashinstall<br>
</code></pre>

<br>
<h2>4.3 Building the AVR tools from source</h2>
Listed below is a set of instructions for building the above AVR tools from sources. This may no longer be necessary as the Arch Linux ARM package repository has the binaries added. (Thanks to Mårten Gustafsson for this contribution).<br>
<pre><code>[root@alarmpi ~]# cd ~<br>
[root@alarmpi ~]# wget http://aur.archlinux.org/packages/av/avr-binutils-atmel/avr-binutils-atmel.tar.gz<br>
[root@alarmpi ~]# tar xvf avr-binutils-atmel.tar.gz<br>
[root@alarmpi ~]# cd avr-binutils-atmel<br>
[root@alarmpi avr-binutils-atmel]# nano PKGBUILD<br>
[root@alarmpi avr-binutils-atmel]# grep arch= PKGBUILD<br>
arch=('i686' 'x86_64' 'armv6h')<br>
[root@alarmpi avr-binutils-atmel]# makepkg -s -–asroot<br>
[root@alarmpi avr-binutils-atmel]# pacman -U avr-binutils-atmel-2.20.1-2-armv6h.pkg.tar.xz<br>
[root@alarmpi avr-binutils-atmel]# cd ..<br>
[root@alarmpi ~]# rm -R avr-binutils-atmel<br>
<br>
[root@alarmpi ~]# wget http://aur.archlinux.org/packages/av/avr-gcc-atmel/avr-gcc-atmel.tar.gz<br>
[root@alarmpi ~]# tar xvf avr-gcc-atmel.tar.gz<br>
[root@alarmpi ~]# cd avr-gcc-atmel<br>
[root@alarmpi avr-gcc-atmel]# nano PKGBUILD<br>
[root@alarmpi avr-gcc-atmel]# grep arch PKGBUILD | grep -v '#'<br>
arch=('i686' 'x86_64' 'armv6h')<br>
[root@alarmpi avr-gcc-atmel]# makepkg -s –asroot<br>
[root@alarmpi avr-gcc-atmel]# pacman -U avr-gcc-atmel-4.5.3-1-armv6h.pkg.tar.xz<br>
[root@alarmpi avr-gcc-atmel]# cd ..<br>
[root@alarmpi ~]# rm -R avr-gcc-atmel<br>
<br>
[root@alarmpi ~]# wget http://aur.archlinux.org/packages/av/avr-libc-atmel/avr-libc-atmel.tar.gz<br>
[root@alarmpi ~]# tar xvf avr-libc-atmel.tar.gz<br>
[root@alarmpi ~]# cd avr-libc-atmel<br>
[root@alarmpi avr-libc-atmel]# makepkg -s --asroot<br>
[root@alarmpi avr-libc-atmel]# pacman -U avr-libc-atmel-1.7.1-1-any.pkg.tar.xz<br>
[root@alarmpi avr-gcc-atmel]# cd ..<br>
[root@alarmpi ~]# rm -R avr-libc-atmel<br>
<br>
[root@alarmpi ~]# svn checkout http://raspy-juice.googlecode.com/svn/trunk/ raspy-juice-read-only<br>
[root@alarmpi ~]# cd raspy-juice-read-only/bootloader/razzor-twiboot-ca2a0a9/linux/<br>
[root@alarmpi linux]# cp twiboot /usr/local/bin/<br>
[root@alarmpi linux]# cd ~/raspy-juice-read-only/firmware/<br>
[root@alarmpi firmware]# make<br>
[root@alarmpi firmware]# make flashinstall<br>
<br>
</code></pre>