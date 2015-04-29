# 1. Introduction #

---

Raspy Juice is an experimental expansion board to supply a Raspberry Pi (**RPi**) host computer with a regulated +5V from a wide-range voltage source. The board also contains a real-time clock, an RS232-level translator for the host console serial port, and an expansion AVR ATmega168A  microcontroller (MCU). This MCU provides the service of controlling 4 channels of RC servo outputs, an RS485 interface, and a half-duplex software-based RS232 interface. In addition, the spare pins of the MCU are brought to an expansion header which may be used for other purposes. The MCU itself is interfaced to the host computer through an I2C/TWI interface, in which the former is a slave device.
<br>
<br>

<h1>2. Details</h1>
<hr />
<br>
<h3>Block Diagram</h3>
<img src='http://raspy-juice.googlecode.com/svn/wiki/juice-block-640x480.png' border='2' />
<br>
<br>

<h3>2.1 Main Features</h3>
<ul><li>6-23V input to 5V, 3A high-efficiency (>80%) buck regulator with input polarity protection.<br>
The main feature of this expansion board is to supply the RPi host computer with a regulated +5V through the GPIO header. With the wide voltage input range, the RPi can be powered from a wide variety of external sources such as batteries, 12V power adapters, solar battery sources, etc. Additional +5V power outputs are also available at the pins of the JST servo ports connectors.<br>
</li><li>RS232-level translated RPi console port with either 2.5mm stereo jack or JST connector.<br>
</li><li>RTC based on NXP PCF8523 with backup power capacitor.</li></ul>

<h3>2.2 Secondary Features</h3>
<ul><li>Atmel ATmega168A AVR microcontroller, running at +3.3V, 14.7456MHz and as a slave I2C device to the RPi host computer.<br>
</li><li>Secondary +3.3 450mA high-efficiency buck regulator<br>
</li><li>4-channel RC servo port with JST connector<br>
</li><li>RS485 interface to USART0 of the AVR microcontroller.<br>
</li><li>RS232 interface to the programmable pins of the AVR microcontroller.<br>
</li><li>Expansion header of unused programmable pins of the microcontroller.</li></ul>

<h3>2.3 Schematics and Drawings</h3>
<ul><li><a href='http://raspy-juice.googlecode.com/svn/wiki/juice-r1-wiki-schem.pdf'>Schematic of Raspy Juice Rev.1 Beta PCBA</a>
</li><li><a href='http://raspy-juice.googlecode.com/svn/wiki/juice-r2-wiki-schem.pdf'>Schematic of Raspy Juice Rev.2 PCBA</a>
</li><li><a href='http://raspy-juice.googlecode.com/svn/wiki/juice-r1-wiki-dim-rpi.pdf'>Mechanical overlay drawing of Raspy Juice over Raspberry Pi</a>
<br>
<br>
<h1>3. Module Connections</h1>
<img src='http://raspy-juice.googlecode.com/svn/wiki/juice-r1-conndiag.png' /></li></ul>

<h3>3.1 RPi GPIO header</h3>
See description on <a href='http://elinux.org/Rpi_Low-level_peripherals'>http://elinux.org/Rpi_Low-level_peripherals</a>

Raspy Juice supplies the host Raspberry Pi with regulated +5V through this GPIO header using a 2A poly-resettable (PTC) fuse. <b>Warning: do not connect a +5V supply through the Raspberry Pi micro-USB connector when used with Raspy Juice.</b>


<h3>3.2 DC Power connector</h3>
<table><thead><th> <b>Pin No.</b> </th><th> <b>Pin Name</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> 1 </td><td> GND  </td><td> Ground connection </td></tr>
<tr><td> 2 </td><td> +VIN </td><td> 6 - 23V positive input, reverse-polarity protected. </td></tr></tbody></table>


<h3>3.3 Console RS232 port and jack</h3>
The two connectors are wired in parallel and only one of these connectors may be used at any time. These ports provide for an RS232 level-translated of the Raspberry Pi console serial port signals (from the above GPIO header), to connect to a laptop, computer or other embedded devices.<br>
<br>
<h5>3.3.1 JST connector</h5>
<table><thead><th> <b>Pin No.</b> </th><th> <b>Pin Name</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> 1 </td><td> GND </td><td> Ground connection </td></tr>
<tr><td> 2 </td><td> CON-TX  </td><td> RS232-level RPi console TX output signal. </td></tr>
<tr><td> 3 </td><td> CON-RX  </td><td> RS232-level RPi console RX input signal. </td></tr></tbody></table>

<h5>3.3.2 Stereo 2.5mm jack</h5>
<table><thead><th> <b>Pin No.</b> </th><th> <b>Pin Name</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> Ring   </td><td> GND </td><td> Ground connection </td></tr>
<tr><td> Tip    </td><td> CON-TX  </td><td> RS232-level RPi console TX output signal. </td></tr>
<tr><td> Middle </td><td> CON-RX  </td><td> RS232-level RPi console RX input signal. </td></tr></tbody></table>


<h3>3.4 AVR RS232 port</h3>
From the expansion AVR microcontroller using a software-emulated UART, this port provides an RS232-level interface for connection to a computer, or other embedded devices. The transmission and reception of the data is through an application firmware service in the microcontroller, and the host Raspberry Pi interacts with the firmware service via the I2C interface of  the GPIO header.<br>
<br>
<table><thead><th> <b>Pin No.</b> </th><th> <b>Pin Name</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> 1 </td><td> GND </td><td> Ground connection </td></tr>
<tr><td> 2 </td><td> AVR-232TX  </td><td> RS232-level AVR TX output signal. Connected and translated from AVR pin PD3. </td></tr>
<tr><td> 3 </td><td> AVR-232RX  </td><td> RS232-level AVR RX input signal. Connected and translated to AVR pin PD2/INT0. </td></tr></tbody></table>

<h3>3.5 AVR RS485 port</h3>
From the expansion AVR microcontroller using the USART0, this port provides an RS485-level half-duplex interface for connection to other RS485-capable devices. No RS485 termination is provided at the PCB board level (ie., pullups/pulldowns/impedance-control) -- these may be configured at the last-mile connector. The transmission and reception of the data is through an application firmware service in the microcontroller, and the host Raspberry Pi interacts with the firmware service via the GPIO header I2C interface. The AVR pin PD4 controls the transmission-enable signal of the RS485 transceiver interface.<br>
<br>
<table><thead><th> <b>Pin No.</b> </th><th> <b>Pin Name</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> 1 </td><td> GND </td><td> Ground connection </td></tr>
<tr><td> 2 </td><td> AVR-485A  </td><td> RS485-level A input/output signal. </td></tr>
<tr><td> 3 </td><td> AVR-485B  </td><td> RS485-level B input/output signal. </td></tr></tbody></table>

<h3>3.6 Servo [1..4] Ports</h3>
From the expansion AVR microcontroller PC[0..3] pins using TIMER1, these ports provide PWM signals of 1 - 2ms pulses, to connect to standard RC servos. The control of the individual PWM is through an application firmware service in the microcontroller, and the host Raspberry Pi interacts with the firmware service via the GPIO header I2C interface. A total of +5V 2A supplies these ports through a poly-resettable (PTC) fuse. Alternatively, these ports can be used to supply +5V to other external circuitry, or embedded devices.<br>
<br>
<table><thead><th> <b>Pin No.</b> </th><th> <b>Pin Name</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> 1 </td><td> GND     </td><td> Ground connection </td></tr>
<tr><td> 2 </td><td> +5V     </td><td> +5V servo power supply output </td></tr>
<tr><td> 3 </td><td> S[1..4] </td><td> Servo control signal at +3.3V logic levels </td></tr></tbody></table>


<h3>3.7 AVR ICSP programming port</h3>
This is the standard Atmel(c) AVR 6-pin programming header.<br>
<br>
<table><thead><th> <b>Description</b> </th><th> <b>Pin</b> </th><th> <b>Pin</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> MISO - communications to an AVR In-System Programmer </td><td> 1 </td><td> 2 </td><td> VCC - Target power </td></tr>
<tr><td> SCK  - communications to an AVR In-System Programmer </td><td> 3 </td><td> 4 </td><td> MOSI - communications to an AVR In-System Programmer </td></tr>
<tr><td> RESET - Target AVR microcontroller reset signal      </td><td> 5 </td><td> 6 </td><td> GND - Ground connection </td></tr></tbody></table>

<h3>3.8 I2C Debug header</h3>
Originally intended for debugging, this unstuffed header may be used for I2C expansion. The +3.3V output power source pin is supplied by the Raspy Juice on-board +3.3V regulator, and a total of 450mA (for the AVR microcontroller, AVR expansion header +3.3V power output pin and this pin) is available.<br>
<table><thead><th> <b>Pin No.</b> </th><th> <b>Pin Name</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> 1 </td><td> +3V3 output</td><td> +3V3 power output source. (Total of 450mA available, //see text//). </td></tr>
<tr><td> 2 </td><td> RPI-SDA </td><td> Directly connected to the above RPi GPIO header (pin 3) SDA signal. </td></tr>
<tr><td> 3 </td><td> RPI-SCL </td><td> Directly connected to the above RPi GPIO header (pin 5) SCL signal. </td></tr>
<tr><td> 4 </td><td> RPI-GPIO4 </td><td> Directly connected to the above RPi GPIO header (pin 7) GPIO4 signal. </td></tr>
<tr><td> 5 </td><td> GND      </td><td> Ground connection.  </td></tr></tbody></table>

<h3>3.9 AVR Expansion header</h3>
Unused or spare pins of AVR microcontroller are routed here.<br>
FixMe:<br>
<ul><li>All signals here are series-terminated with 470ohm resistors.<br>
</li><li>PB[0..3] and PD[5..6]  are +5v over-voltage protected with diodes.<br>
</li><li>ADC[6..7] are +5V tolerant.<br>
</li><li>sub-schematics here.</li></ul>

<table><thead><th> <b>Description</b> </th><th> <b>Pin</b> </th><th> <b>Pin</b> </th><th> <b>Description</b> </th></thead><tbody>
<tr><td> GND  </td><td> 1 </td><td> 2  </td><td> +3V3 </td></tr>
<tr><td> ADC6 </td><td> 3 </td><td> 4  </td><td> PB3  </td></tr>
<tr><td> ADC7 </td><td> 5 </td><td> 6  </td><td> PB2  </td></tr>
<tr><td> PD6  </td><td> 7 </td><td> 8  </td><td> PB1  </td></tr>
<tr><td> PD5  </td><td> 9 </td><td> 10 </td><td> PB0  </td></tr>