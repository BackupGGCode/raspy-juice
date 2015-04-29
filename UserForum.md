# Introduction #

This is possibly a temporary page until we find a better forum.

# Source code #

Subversion is great, but would it be possible to consider moving to [Github](http://github.com/)? It makes it a lot easier to manage contributions.

# Interrupt-driven ADC experiments #

I want to configure my Raspy Juice to monitor the two ADC pins, and signal the RPi via AVR-PD7 whenever the state changes. The pins will be connected to a set of control panel switches configured as a resistor ladder (it's an 80's radio / CD / tape player - an Aiwa CSD-EL50).

I set up the ADC in free running mode, with an ISR which stores the 10-bit value in a volatile array, and switches between ADC6 and ADC7. I added an extra pair of commands to the main TWI code to retrieve the values.

It works fine but after a few minutes all TWI commands start failing until the RJ is rebooted. I rolled back to the original firmware and found that the same thing happens. Conclusion - the TWI code is not very robust, maybe it would work better to drop in Atmel's TWI\_Slave.{c,h} and use their routines?