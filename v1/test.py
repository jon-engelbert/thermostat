#!/usr/bin/python

import wiringpi2
import time

time.sleep(15)

delay = 0.5
gpio = wiringpi2.GPIO(wiringpi2.GPIO.WPI_MODE_GPIO)
gpio.pinMode(27, gpio.OUTPUT)
gpio.pinMode(22, gpio.OUTPUT)
gpio.pinMode(18, gpio.OUTPUT)
gpio.pinMode(17, gpio.OUTPUT)
gpio.pinMode(15, gpio.OUTPUT)
gpio.pinMode(14, gpio.OUTPUT)

while (True):
	gpio.digitalWrite(27, gpio.LOW)
	time.sleep(delay)
	gpio.digitalWrite(22, gpio.LOW)
	time.sleep(delay)
	gpio.digitalWrite(18, gpio.LOW)
	time.sleep(delay)
	gpio.digitalWrite(17, gpio.HIGH)
	time.sleep(delay)
	gpio.digitalWrite(15, gpio.HIGH)
	time.sleep(delay)
	gpio.digitalWrite(14, gpio.HIGH)

#	time.sleep(delay)
	time.sleep(delay * 2.0)

	gpio.digitalWrite(27, gpio.HIGH)
	time.sleep(delay)
	gpio.digitalWrite(22, gpio.HIGH)
	time.sleep(delay)
	gpio.digitalWrite(18, gpio.HIGH)
	time.sleep(delay)
	gpio.digitalWrite(17, gpio.LOW)
	time.sleep(delay)
	gpio.digitalWrite(15, gpio.LOW)
	time.sleep(delay)
	gpio.digitalWrite(14, gpio.LOW)

#	time.sleep(delay)
	time.sleep(delay * 2.0)
