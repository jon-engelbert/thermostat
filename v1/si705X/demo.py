#!/usr/bin/python

import si705X
import time

si705X.startup(1, 0x40)
si705X.reset()

rev = si705X.get_revision()
print "Revision = {0:#0x}".format(rev)

esn1,esn2 = si705X.get_ESN()
print "ESN = {0:#06x} {1:04x}".format(esn1, esn2)

reg1 = si705X.get_reg1()
print "Reg1 = {0:#02x}".format(reg1)

print "Starting 14-bit measurement"
temperature = si705X.get_tempC()
print "Temperature = {0} degrees C".format(temperature)

si705X.set_reg1((0x7E & reg1) | 0x80)
reg1 = si705X.get_reg1()
print "Reg1 = {0:#02x}".format(reg1)

print "Starting 13-bit measurement"
temperature = si705X.get_tempC()
print "Temperature = {0} degrees C".format(temperature)

si705X.set_reg1((0x7E & reg1) | 0x01)
reg1 = si705X.get_reg1()
print "Reg1 = {0:#02x}".format(reg1)

print "Starting 12-bit measurement"
temperature = si705X.get_tempC()
print "Temperature = {0} degrees C".format(temperature)

si705X.set_reg1(0x81 | reg1)
reg1 = si705X.get_reg1()
print "Reg1 = {0:#02x}".format(reg1)

print "Starting 11-bit measurement"
temperature = si705X.get_tempC()
print "Temperature = {0} degrees C".format(temperature)

si705X.set_reg1(0x7E & reg1)
reg1 = si705X.get_reg1()
print "Reg1 = {0:#02x}".format(reg1)

while(1):
    temperature = si705X.get_tempF()
    print "Temperature = {0:>3.1f} degrees F{1:1c}[1A".format(temperature, 27)
