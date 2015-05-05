#!/usr/bin/python

import pcf8523
import si705X
import wiringpi2
import atexit
import cPickle as pickle
import json
import codecs
import errno
import fnmatch
import io
import os
import sys
import signal
import pygame
from pygame.locals import *
from subprocess import call  
import time
import datetime
import syslog
import threading
import calendar

# Globals

DEBUG = True

iconPath = 'icons'
icons = []
screenMode = "main"
screenModePrior = "none"

current_temp = 72.0;

lastfont = None
lastfontname = ''
lastfontsize = 16

relays = {
    "W1": { "gpio": 17, "pin": 11 },
    "W2": { "gpio": 15, "pin": 10 },
    "W3": { "gpio": 14, "pin":  8 },
    "G" : { "gpio": 18, "pin": 12 },
    "Y1": { "gpio": 27, "pin": 13 },
    "Y2": { "gpio": 22, "pin": 15 }
}

v = {
       "bright_threshold": 60, 
       "mode-24hour": False, 
       "FontIndex": 51, 
       "leading-zero": False,
       "temperature_in_F": True
    }

class Icon:
    def __init__(self, name):
      self.name = name
      try:
        self.bitmap = pygame.image.load(iconPath + '/' + name + '.png')
      except:
        pass

class Button:
    def __init__(self, rect, **kwargs):
        self.rect     = rect # Bounds
        self.color    = None # Background fill color, if any
        self.iconBg   = None # Background Icon (atop color fill)
        self.iconFg   = None # Foreground Icon (atop background)
        self.bg       = None # Background Icon name
        self.fg       = None # Foreground Icon name
        self.callback = None # Callback function
        self.value    = None # Value passed to callback
        for key, value in kwargs.iteritems():
            if   key == 'color': self.color    = value
            elif key == 'bg'   : self.bg       = value
            elif key == 'fg'   : self.fg       = value
            elif key == 'cb'   : self.callback = value
            elif key == 'value': self.value    = value

    def selected(self, pos):
        x1 = self.rect[0]
        y1 = self.rect[1]
        x2 = x1 + self.rect[2] - 1
        y2 = y1 + self.rect[3] - 1
        if ((pos[0] >= x1) and (pos[0] <= x2) and (pos[1] >= y1) and (pos[1] <= y2)):
            if self.callback:
                if self.value is None:
                    self.callback(self)
                else:
                    self.callback(self, self.value)
            return True
        return False

    def draw(self, screen):
        if self.bg is not None:
            for i in icons:      #   For each icon...
                if self.bg == i.name: #    Compare names; match?
                    self.iconBg = i     #     Assign Icon to Button
                    self.bg     = None  #     Name no longer used; allow garbage collection
                    break
        if self.fg is not None:
            for i in icons:      #   For each icon...
                if self.fg == i.name:
                    self.iconFg = i
                    self.fg     = None
                    break
        if self.color:
            screen.fill(self.color, self.rect)
        if self.iconBg:
            screen.blit(self.iconBg.bitmap,
                (self.rect[0]+(self.rect[2]-self.iconBg.bitmap.get_width())/2,
                self.rect[1]+(self.rect[3]-self.iconBg.bitmap.get_height())/2))
        if self.iconFg:
            screen.blit(self.iconFg.bitmap,
                (self.rect[0]+(self.rect[2]-self.iconFg.bitmap.get_width())/2,
                self.rect[1]+(self.rect[3]-self.iconFg.bitmap.get_height())/2))

    def setBg(self, name):
        if name is None:
            self.iconBg = None
        else:
            for i in icons:
                if name == i.name:
                    self.iconBg = i
                    break

class Panel:
    
    buttons = [];
    
    def __init__(self, name, isstatic, draw_func, buttons):
        self.name = name
        self.isstatic = isstatic
        self.draw_func = draw_func
        self.buttons = buttons

    def draw(self, screen):
        # Overlay buttons on display and update
        for i,b in enumerate(self.buttons):
            b.draw(screen)
        if self.draw_func:
            self.draw_func(self)

###############################################################################################################

def screen_main_cb(button, n): # normal display
    global screenMode
    global v
    
    if n == 1:   # config
        screenMode = "main"
    else:
        screenMode = "duh" # nothing else yet

def screen_main_draw(panel): # normal display
    global screenMode
    global v
    
    fontsugsz0 = 12
    
    z = "{0:2.1f}".format(get_display_temp())
    try:
        fontsugsz0 = centerMaxText(screen, z, fgcolor, (0, 70, 320, 160), allfonts[v['FontIndex']], fontsugsz0)
    except IndexError:
        v['FontIndex'] = 1
        fontsugsz0 = centerMaxText(screen, z, fgcolor, (0, 70, 320, 160), allfonts[0], fontsugsz0)

def screen_duh_draw(panel): # normal display
    global screenMode
    global v
    
    fontsugsz0 = 12
    
    z = get_display_time(datetime.datetime.now())
    try:
        fontsugsz0 = centerMaxText(screen, z, fgcolor, (0, 70, 320, 160), allfonts[v['FontIndex']], fontsugsz0)
    except IndexError:
        v['FontIndex'] = 1
        fontsugsz0 = centerMaxText(screen, z, fgcolor, (0, 70, 320, 160), allfonts[0], fontsugsz0)

###############################################################################################################

panels = {
    "main": Panel("main", False, screen_main_draw, 
        [Button((0,  0, 320,  60), bg='box', color=(48,48,48), cb=screen_main_cb, value=1),
        Button((0, 60, 320, 180), bg='box', color=(32,32,32), cb=screen_main_cb, value=2)]),
    "duh": Panel("duh", False, screen_duh_draw, 
        [Button((0,  0, 320,  60), bg='box', color=(48,48,48), cb=screen_main_cb, value=1),
        Button((0, 60, 320, 180), bg='box', color=(32,32,32), cb=screen_main_cb, value=2)])
}

###############################################################################################################
# Assorted utility functions -----------------------------------------------
###############################################################################################################

def log_info(msg):
    syslog.syslog(syslog.LOG_INFO, msg)
    if DEBUG: print msg

def log_error(msg):
    syslog.syslog(syslog.LOG_ERR, msg)
    if DEBUG: print msg

def killitwithfire(n, stack):
    if n == signal.SIGINT:    
        log_info("SIGNAL: Program halted with ^C")
    elif n == signal.SIGTERM:
        log_info("SIGNAL: Program Terminated")
    elif n == signal.SIGQUIT:
        log_info("SIGNAL: Program Quit")
    sys.exit(0)
    
def handle_sighup(n, stack):
    log_info("Reading persistent configuration variables")
    read_persistent_vars()

def write_persistent_vars():
    with open('/etc/{0}.conf'.format(os.path.splitext(os.path.basename(__file__))[0]), 'w') as output:
        json.dump(v, output, indent=3)
        output.flush()
        output.close()

def read_persistent_vars():
    global v
    global config_last_read
    
    try:
        with codecs.open('/etc/{0}.conf'.format(os.path.splitext(os.path.basename(__file__))[0]), 'r', encoding='utf-8') as duhput:
            v = json.load(duhput)
    except IOError:
        write_persistent_vars()
    config_last_read = datetime.datetime.now()

def get_display_temp():
    global v
    global current_temp
    
    if v.get('temperature_in_F', True): # Fahrenheit
        current_temp = si705X.get_tempF()
    else: # Centigrade
        current_temp = si705X.get_tempC()
    return current_temp

def get_display_time(when):
    global v

    hour = when.hour
    if not v.get('mode-24hour', False): # 12-hour AM/PM mode
        hour = hour if hour <= 12 else hour - 12
        hour = hour if hour > 0 else 12
        if v.get('leading-zero', False): # leading zero?
            z = "{0:02}:{1:02}".format(hour, when.minute)
        else:
            z = "{0}:{1:02}".format(hour, when.minute)
    else: # 24-hour mode
        z = "{0:02}:{1:02}".format(hour, when.minute)
    return z

def centerMaxText(surface, text, color, rectTP, fontname, fontsizesuggestion, coast=True, aa=True, bkg=None):
    global lastfont
    global lastfontname
    global lastfontsize

    rect = Rect(rectTP)
    fontsize = fontsizesuggestion
    if fontname == lastfontname and fontsize == lastfontsize:
        font = lastfont
    else:
        # this operation costs file I/O
        font = pygame.font.SysFont(fontname, fontsize)
        
        # determine minimum
        while font.size(text)[1] < rect.height or font.size(text)[0] < rect.width:
            fontsize += 2
            font = pygame.font.SysFont(fontname, fontsize)
            if not coast: break
        
        # determine maximum
        while font.size(text)[1] >= rect.height or font.size(text)[0] >= rect.width:
            fontsize -= 2
            font = pygame.font.SysFont(fontname, fontsize)
            if not coast: break
        
        lastfontname = fontname  
        lastfontsize = fontsize
        lastfont = font

    # render the line and blit it to the surface
    if bkg:
        image = font.render(text, 1, color, bkg)
        image.set_colorkey(bkg)
    else:
        image = font.render(text, aa, color)

    #surface.blit(image, (rect.left, rect.top))
    surface.blit(image,
      (rect.left + (rect.width - image.get_width() ) / 2,
       rect.top + (rect.height - image.get_height()) / 2))

    return fontsize

###############################################################################################################
###############################################################################################################
###############################################################################################################
###############################################################################################################

if __name__ == '__main__':

    log_info("Hello, I am a computer.  Whir, click, beep!")
    pid = str(os.getpid())
    pidfile = os.path.join("/var", "run", os.path.splitext(os.path.basename(__file__))[0]+".pid")
    
    if os.path.isfile(pidfile):
        log_error("%s already exists, exiting" % pidfile)
        sys.exit(2)

    with file(pidfile, 'w') as f:
        f.write(pid)
        f.flush()
        f.close()

    # Initialization -----------------------------------------------------------
    log_info("Initializing...")
    read_persistent_vars()

    # Init framebuffer/touchscreen environment variables
    os.putenv('SDL_VIDEODRIVER', 'fbcon')
    os.putenv('SDL_FBDEV'      , '/dev/fb1')
    os.putenv('SDL_MOUSEDRV'   , 'TSLIB')
    os.putenv('SDL_MOUSEDEV'   , '/dev/input/touchscreen')
    
    # Init pygame and screen
    pygame.init()
    log_info("Setting Mouse invisible...")
    pygame.mouse.set_visible(False)
    log_info("Setting fullscreen...")
    modes = pygame.display.list_modes(16, pygame.FULLSCREEN)
    screen = pygame.display.set_mode(modes[0], pygame.FULLSCREEN, 16)

    log_info("Loading Fonts...")
    allfonts = pygame.font.get_fonts()
    
    log_info("Loading Icons...")

    # Load all icons at startup.
    for file in os.listdir(iconPath):
        if fnmatch.fnmatch(file, '*.png'):
            icons.append(Icon(file.split('.')[0]))

    # Assign Icons to Buttons, now that they're loaded
    log_info("Assigning Buttons")
    for p in panels.values():
        for b in p.buttons:
            for i in icons:
                if b.bg == i.name:
                    b.iconBg = i # Assign Icon to Button
                    b.bg = None # Name no longer used; allow garbage collection
                if b.fg == i.name:
                    b.iconFg = i
                    b.fg = None
    
    # Set up GPIO pins
    log_info("Initializing GPIO pins...")
    gpio = wiringpi2.GPIO(wiringpi2.GPIO.WPI_MODE_GPIO)
    for relay in relays.values():
        gpio.pinMode(relay.get('gpio'), gpio.OUTPUT)
        gpio.pullUpDnControl(relay.get('gpio'), gpio.PUD_DOWN)
        gpio.digitalWrite(relay.get('gpio'), 0)

    log_info("Initializing temperature sensor...")
    si705X.startup(1, 0x40)
    si705X.reset()
    si705X.set_reg1(0x7E) # 14-bit

    log_info("loading background")
    try:
        img = pygame.image.load("icons/bg.png")
    except:
        img = None
    
    if img is None or img.get_height() < 240: # Letterbox, clear background
        screen.fill(0)
    if img:
        screen.blit(img, ((320 - img.get_width() ) / 2, (240 - img.get_height()) / 2))
    pygame.display.flip()
    
    calendar.setfirstweekday(calendar.SUNDAY)

    signal.signal(signal.SIGINT, killitwithfire)
    signal.signal(signal.SIGQUIT, killitwithfire)
    signal.signal(signal.SIGTERM, killitwithfire)
    signal.signal(signal.SIGHUP, handle_sighup)
    
###############################################################################################################
# Main loop ------------------------------------------------------------------
###############################################################################################################
    
    fontsugsz41 = 16
    fontsugsz42 = 16
    fontsugsz0 = 12
    fontsugszA = 12
    
    arial50 = pygame.font.SysFont("Arial", 50)
    font_arial_Small = pygame.font.SysFont("Arial", 24)
    font_fixed_Small = pygame.font.SysFont("fixed", 24)
    fgcolor = (0,0,0)
    bgcolor = (0,0,0)
    
    when = datetime.datetime.now()
    go = True
    log_info("Main loop...")
    try:
        while go:
            # Process touchscreen input
            while go:
                when = datetime.datetime.now()
                for event in pygame.event.get():
                    if (event.type is MOUSEBUTTONDOWN):
                        pos = pygame.mouse.get_pos()
                        for b in panels[screenMode].buttons:
                            if b.selected(pos):
                                break
                    elif (event.type is MOUSEBUTTONUP):
                        pass
                if not panels[screenMode].isstatic or screenMode != screenModePrior:
                    break
                time.sleep(0.01)
            
            fgcolor = (255,255,255)
            bgcolor = (0,0,0)
            screen.fill(bgcolor)

###############################################################################################################

            panels[screenMode].draw(screen)

###############################################################################################################

            pygame.display.flip()
            time.sleep(0.01)
        
            screenModePrior = screenMode

###############################################################################################################

    except SystemExit:
        log_info("System Exiting")
        go = False
    #except:
    #    log_error("Unexpected system error: {0}".format(sys.exc_info()[0]))
    #    go = False
    finally:
        pygame.quit()
        
        try:
            os.unlink(pidfile)
        except:
            pass
        go = False
        log_info("601.")
