joyfab/SunTracker-SmartStepper (C)2018 joyjoy.fr


NanoSunTracker:  
Two axis suntracking controller. by joy.
NanoSunTracker for arduino to run software
Specifics Libraries:
#include <Helios.h>
#include <DS1107H.h>
#include <Wire.h> 
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);


SmartStepper:
L6470: Microstepper (128 µsteps) motor controller. SPI 
AT328: Arduino controller Atmel 
Specifics Libraries:
dSPIN_example.zip Library files:
L6470.h
dSPIN_support.ino
dSPIN_commands.ino
dSPIN_example.ino
