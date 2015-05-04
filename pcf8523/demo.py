#!/usr/bin/python

import pcf8523
import time

pcf8523.startup(1, 0x68)
pcf8523.reset()

pcf8523.set_reg(0x00, 0x00)
pcf8523.set_reg(0x01, 0x00)
pcf8523.set_reg(0x02, 0x00)
pcf8523.set_reg(0x0F, 0x30)

for i in range(0, 20, 1):
    reg = pcf8523.fetch_reg(i)
    print "reg({0:#02x}) = {1:#02x}".format(i, reg)

bat_ok = pcf8523.check_battery()
if (bat_ok > 0):
    print "Battery is OK"
elif (bat_ok < 0):
    print "Battery check failed, unknown result."
else:
    print "Battery is BAD."

yr,mo,dow,dy,ho,mi,sc = pcf8523.fetch_chip_time()

print "The chip says it's: {0} day of week, {1:d}/{2:d}/{3:04d} {4:02d}:{5:02d}:{6:02d}".format(dow, mo, dy, yr+2000, ho, mi, sc)
