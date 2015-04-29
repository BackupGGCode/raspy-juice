# Introduction #

**Raspy Juice** is an experimental expansion board with an AVR ATmega168A microcontroller (MCU) to provide the services of controlling 4 channels of RC servo outputs, an RS485 interface, and a half-duplex software-based RS232 interface. In addition, the spare pins of the MCU is brought to an expansion header which may be used for other purposes. The MCU itself is interfaced to the host computer through an I2C/TWI interface, in which the former is a slave device.

This **_draft_** document is to plan and describe the emulated I2C register set with which the MCU control-firmware is to interprete and perform actions, or reply to the host computer.

# Details #

#### Devices and I2C Addresses on the Raspy Juice Exp Board ####
  * MCU slave   (0x48) I2C write address: 0x90<br>
<ul><li>MCU slave   (0x48) I2C read  address: 0x91<br>
</li><li>RTC PCF8523 (0x68) I2C write address: 0xd0<br>
</li><li>RTC PCF8523 (0x68) I2C read  address: 0xd1<br></li></ul>

<h4>Desired functions the MCU should perform</h4>
<ul><li>Set the 4-channel RC servo outputs<br>
</li><li>Read/write MCU EEPROM<br>
</li><li>Control, read status and read/write to the RS485 interface<br>
</li><li>Control, read status and read/write to the RS232 interface<br>
</li><li>Control, set direction, and read/write to the spare MCU GPIO pins<br>
</li><li>Control, and read the spare MCU ADC pins<br>
</li><li>Reset and activate the MCU in bootloader mode for firmware updates</li></ul>

<h4>Emulated I2C Register Set</h4>

<table><thead><th> <b>Register</b><br>Address</th><th> <b>Name</b> </th><th> <b>Bit7</b> </th><th> <b>Bit 6</b> </th><th> <b>Bit 5</b> </th><th> <b>Bit 4</b> </th><th> <b>Bit 3</b> </th><th> <b>Bit 2</b> </th><th> <b>Bit 1</b> </th><th> <b>Bit 0</b> </th><th> <b>Remarks</b> </th></thead><tbody>
<tr><td> 0x00 </td><td> GSTAT </td><td>  </td><td> ADCB </td><td> EEB </td><td>  </td><td>  </td><td> RXA485 </td><td>  </td><td> RXA232 </td><td>  </td></tr>
<tr><td>  </td><td> GCONT </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td></tr>
<tr><td>  </td></tr>
<tr><td> 0x01 </td><td> SRV_0   </td><td> bit7<br>bit15 </td><td> bit6<br>bit14 </td><td> bit5<br>bit13 </td><td> bit4<br>bit12 </td><td> bit3<br>bit11 </td><td> bit2<br>bit10 </td><td> bit1<br>bit9 </td><td> bit0<br>bit8 </td><td> <b>unsigned short (uint16_t)</b></td></tr>
<tr><td> 0x02 </td><td> SRV_1   </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td> <b>Use I2C word-write to set pulse-width in</b>      </td></tr>
<tr><td> 0x03 </td><td> SRV_2   </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td> <b>microseconds. Limited to 500usec to 2500usec.</b>   </td></tr>
<tr><td> 0x04 </td><td> SRV_3   </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td> <b>RC Servos are usually 1.00ms to 2.00ms.</b> </td></tr>
<tr><td>  </td><td> <font color='#909090'>SRV_4</font>   </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td></tr>
<tr><td>  </td><td> <font color='#909090'>SRV_5</font>   </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td></tr>
<tr><td>  </td><td> <font color='#909090'>SRV_6</font>   </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td></tr>
<tr><td>  </td><td> <font color='#909090'>SRV_7</font>   </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td></tr>
<tr><td>  </td></tr>
<tr><td> 0x10 </td><td> RX232   </td></tr>
<tr><td> 0x10 </td><td> TX232   </td></tr>
<tr><td>  </td><td> <font color='#909090'>RLVL232 </td></tr>
<tr><td>  </td><td> <font color='#909090'>TLVL232 </td></tr>
<tr><td>  </td><td> <font color='#909090'>STAT232 </td></tr>
<tr><td>  </td><td> <font color='#909090'>MCR232  </td></tr>
<tr><td>  </td><td> <font color='#909090'>BPS232  </td></tr>
<tr><td>  </td></tr>
<tr><td> 0x20 </td><td> <font color='#909090'>RX485   </td></tr>
<tr><td> 0x20 </td><td> <font color='#909090'>TX485   </td></tr>
<tr><td>  </td><td> <font color='#909090'>RLVL485 </td></tr>
<tr><td>  </td><td> <font color='#909090'>TLVL485 </td></tr>
<tr><td>  </td><td> <font color='#909090'>STAT485 </td></tr>
<tr><td>  </td><td> <font color='#909090'>MCR485  </td></tr>
<tr><td>  </td><td> <font color='#909090'>BPS485  </td></tr>
<tr><td>  </td></tr>
<tr><td> 0x40 </td><td> ADCMUX </td><td>  </td><td>  </td><td>  </td><td>  </td><td> MUX3 </td><td> MUX2 </td><td> MUX1 </td><td> MUX0 </td><td> <b>Writing initiates conversion</b> </td></tr>
<tr><td> 0x41 </td><td> ADCDAT </td><td> bit7<br>0 </td><td> bit6<br>0 </td><td> bit5<br>0 </td><td> bit4<br>0 </td><td> bit3<br>0 </td><td> bit2<br>0 </td><td> bit1<br>bit9 </td><td> bit0<br>bit8 </td><td>  </td></tr>
<tr><td>  </td></tr>
<tr><td> 0x50 </td><td> EEADDR </td><td> adr7<br>x </td><td> adr6<br>x </td><td> adr5<br>x </td><td> adr4<br>x </td><td> adr3<br>x </td><td> adr2<br>x </td><td> adr1<br>x    </td><td> adr0<br>adr9 </td><td> <b>Writing initiates readback</b><br> into EEDATA</td></tr>
<tr><td> 0x51 </td><td> EEDATA </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td> <b>Writing initiates</b><br>EEPROM programming</td></tr>
<tr><td> 0xb0 </td><td> REBOOT </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td>  </td><td> <b>Writing with 0x0d reboots into bootloader</b> </td></tr>