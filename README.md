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
  * **RTIME**: speed rise time in seconds

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

## Schematics

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/schematic_leonardo.png" width=500>
<img src="https://github.com/micworg/stir/blob/master/images/schematic_uno.png" width=500>
</p>
