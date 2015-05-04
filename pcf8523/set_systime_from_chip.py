#!/usr/bin/python

import pcf8523
import datetime

pcf8523.startup(1, 0x68)

bat_ok = pcf8523.check_battery()
if (bat_ok > 0):
    print "Battery is OK"
elif (bat_ok < 0):
    print "Battery check failed, unknown result."
else:
    print "Battery is BAD."

yr,mo,dow,dy,ho,mi,sc = pcf8523.fetch_chip_time()

print "The chip says it's: {0} day of week, {1:d}/{2:d}/{3:04d} {4:02d}:{5:02d}:{6:02d}".format(dow, mo, dy, yr+2000, ho, mi, sc)

if (pcf8523.set_systime_from_chip() < 0):
    print "Failed to set system time."

when = datetime.datetime.now()

yr = when.year
mo = when.month
dy = when.day
dow = when.weekday()
ho = when.hour
mi = when.minute
sc = when.second

print "The system time is: {0} day of week, {1:d}/{2:d}/{3:04d} {4:02d}:{5:02d}:{6:02d}".format(dow, mo, dy, yr, ho, mi, sc)

