# thermostat (raspistat?)

This is a Raspberry Pi based thermostat.  The word "hat" as applied to a Raspberry Pi
includes an EEPROM with the proper device-tree overlay and driver code on it.  Since
I don't know how to do that yet, this is just a "lid".

The printed circuit board and corresponding drivers and application code are designed
to work with a Raspberry Pi model A+ and a PiTFT frame-buffer resistive touch-screen
LCD display (Adafruit product ID: 1601).  I'm sure other Raspberry Pi models
will also work, and probably any LCD display that can be driven with the PyGame library
at 320 x 240 resolution.

The circuit contains three main elements, wait, 4... four main elements... I'll start again.

The circuit contains four main elements:
* Raspberry Pi power supply (5 volts 1.25 amps) derived from 18 to 24 volts AC, or 9 to 35 volts DC
* six GPIO-controlled mechanical relays configured like a conventional thermostat
* a temperature sensor with good resolution
* a real-time-clock, with backup battery, so the circuit knows the time without internet access

For the full hardware description, see the README.pdf in the hardware folder.

What does it do?

A thermostat is a relatively simple device, it turns on equipment when the temperature exceeds
high or low limits.  There are generally three controls, the setting of the temperature (dial),
a switch for selecting "Heat", "OFF", or "Cool", and a switch for the fan "On" or "Automatic"
(whenever the system is running).  Adding a computer to this allows scheduling of target temperatures,
recording of temperatures and equipment usage, and additional display functions.

What else needs to be said here?  Hmmm.... dependencies...

* WiringPi2 library, available from https://github.com/Gadgetoid/WiringPi2-Python

Please direct inquiries to github-spam-trap@mdve.net

--Roger
