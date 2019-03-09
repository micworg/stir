# Dual magnetic stir controller

## Overview

This is the Arduino sketch to build a double magnetic stirrer based on 2 fans, a 1602 LCD display, a KY-040 encoder and an Arduino (e.g. Uno, Leonardo, Pro Micro).

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/stir.jpg" width=500>
</p>

In order to put the stirrer into operation you have to change the settings in stir.ino according to your setup.

## Overview of Functions

All functions are controlled by the KY-040 encoder:

* **Press briefly:** change between menu, stirrer 1 and stirrer 2
* **Turn when menu is selected**: change between menu items (SPEED, BOOST, BTIME, CATCH, CTIME)
* **Long press when menu is selected**: lock all functions (unlock also by long press)
* **When a stirrer is selected**:
  * **SPEED**: turn sets speed, long press switches stirrer on or off.
  * **BOOST**: turn sets boost speed, long press activates/deactivates the boost function (running time is shown in the display).
  * **BTIME**: turn sets the boost time in minutes.
  * **CATCH**: turn activates/deactivates the fishing function
  * **CTIME**: turn sets the interval in minutes for the fish catching function
  * **RTIME**: speed rise time in seconds (applys to power on, stirrer on, boost on and fish catching)

## Display Indicators:

* **Bottom Left**: Menu
* **Bottom centre/right**: displays the set values for the stirrers depending on the selected menu item.
* **Top Center/Right**: displays the current stirrer speed (or 'OFF' or 'CAT' (Fishing)).
* **Top left**: shows the remaining time of the boost function.

## Settings (in stir.ino)

|Value|Description|
|:----|:----------|
|SPEEDINC|increment of speed values when turning the encoder|
|FANMIN|fan minimum speed (should be a value at which the fan runs safely) (rpm)|
|FANMAX|maximum speed of the fan (it is important that this is the real maximum value of the fan) (rpm)|
|CATCHSTOP|stop interval for stir fish catch function (ms)|
|PWM0, PWN1|PWM output pins|
|I0, I1|interrupts for rpm measurement (2 and 3 for Leonardo and ProMicro / 0 and 1 for Uno)|
|CLK, DT, SW|pins for KY-040 encoder| 
|RINTERVAL|regulation interval (ms)|
|RDELAY|extra regulation delay when value changes (ms)|
|RTOL|regulation tolerance (rpm)|
|SINTERVAL|speed measurement interval (ms)|
|SAVERAGE|speed measurement average|
|SAVETAG|parameter save identifier|
|SAVEDELAY|delay in seconds before parameter will be saved to EEPROM|

## API commands

Command are colon separated an can be send via USB/Serial

|Command|Description|
|:------|:----------|
|info|returns a colon separated string with all parameters (see below)|
|version|returns the software version|
|on:<0/1>|switch stirren on|
|off:<0/1>|switch stirren off|
|bon:<0/1>|switch boost mode on|
|boff:<0/1>|switch boost mode off|
|speed:<0/1>:<speed>|set stirrer speed|
|bspeed:<0/1>:<speed>|set stirrer boost speed|
|btime:<0/1>:<time>|set boost time|
|con:<0/1>|switch catch mode on|
|coff:<0/1>|switch catch mode off|
|ctime:<0/1>:<time>|set catch mode interval|
|rtime:<0/1>:<time>|set speed rise time|

All commands return a colon separated string with all current parameters:

|Element|Descriptoion|
|:------|:-----------|
|0|stirrer 0 state|
|1|stirrer 0 speed|
|2|stirrer 0 boost speed|
|3|stirrer 0 rpm|
|4|stirrer 0 averaged rpm|
|5|stirrer 0 regulation value|
|6|stirrer 0 boost time|
|7|stirrer 0 fish catch mode state|
|8|stirrer 0 fish catch mode time|
|9|stirrer 0 speed rise time|
|10|stirrer 1 state|
|11|stirrer 1 speed|
|12|stirrer 1 boost speed|
|13|stirrer 1 rpm|
|14|stirrer 1 averaged rpm|
|15|stirrer 1 regulation value|
|16|stirrer 1 boost time|
|17|stirrer 1 fish catch mode state|
|18|stirrer 1 fish catch mode time|
|19|stirrer 1 speed rise time|
|20|0=ok, 1=error|
|21|software version|

## Schematics

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/schematic_leonardo.png" width=500>
<img src="https://github.com/micworg/stir/blob/master/images/schematic_uno.png" width=500>
</p>
