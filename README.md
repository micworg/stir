# Dual magnetic stir controller

## Overview

This is the Arduino sketch to build a double magnetic stirrer based on 2 fans, a 1602 LCD display, a KY-040 encoder and an Arduino (e.g. Uno, Leonardo, Pro Micro).

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/stir.jpg" width=500>
</p>

In order to put the stirrer into operation you have to change the settings in stir.ino according to your setup.

## Overview of Functions

All functions are controlled by the KY-040 encoder:

* **Short Press:** change between menu, stirrer 1 and stirrer 2
* **Turn when menu is selected**: change between menu items (SPEED, BOOST, BTIME, CATCH, CTIME)
* **Long press when menu is selected**: lock all functions (unlock also by long press)
* **When a stirrer is selected**:
  * **SPEED**: turn sets speed, long press switches stirrer on or off.
  * **BOOST**: turn sets boost speed, long press activates/deactivates the boost function (running time is shown in the display).
  * **BTIME**: turn changes the boost time in minutes.
  * **CATCH**: turn activates/deactivates the fishing function
  * **CTIME**: turn changes the interval in minutes for the fish catching function
  * **RTIME**: turn changes speed rise time in seconds (applies to power on, stirrer on, boost on and fish catching)
  * **OTIME**: turn changes switch off time in hours (the timer activates immediately, 0 = switch off disabled)
* **Other menu options**:
  * **BRGHT**: LCD brightness

## Display Indicators:

* **Bottom Left**: Menu
* **Bottom Center/Right**: displays the set values for the stirrers depending on the selected menu item.
* **Top Center/Right**: displays the current stirrer speed (or 'OFF' or 'CAT' (Fishing)).
* **Top Left**: shows the remaining time of the boost or switch off function.

## Settings (in stir.ino)

|Value|Description|
|:----|:----------|
|`SPEEDINC`|increment of speed values when turning the encoder|
|`FANMIN`|fan minimum speed (should be a value at which the fan runs safely) (rpm)|
|`FANMAX`|maximum speed of the fan (it is important that this is the real maximum value of the fan) (rpm)|
|`BOFF`|if set boost off will also turn the normal mode off|
|`BINC`|BTIME increment (min)|
|`CINC`|CTIME increment (min)|
|`RINC`|RTIME increment (sec)|
|`OINC`|OTIME increment (hour)|
|`CATCHSTOP`|stop interval for stir fish catch function (ms)|
|`PWM0`, `PWN1`|PWM output pins|
|`I0`, `I1`|interrupts for rpm measurement (2 and 3 for Leonardo and ProMicro / 0 and 1 for Uno)|
|`CLK`, `DT`, `SW`|pins for KY-040 encoder| 
|`OFF0`, `OFF1`|these pins will be set if the fan is off| 
|`OFFSTATE`|off state (LOW/HIGH)| 
|`RINTERVAL`|regulation interval (ms)|
|`RDELAY`|extra regulation delay when value changes (ms)|
|`RTOL`|regulation tolerance (rpm)|
|`SINTERVAL`|speed measurement interval (ms)|
|`SAVERAGE`|speed measurement average|
|`SAVETAG`|parameter save identifier|
|`SAVEDELAY`|delay in seconds before parameter will be saved to EEPROM|
|`LCDB`|LCD brightness steps (10 values, 0=off, 255=max)|

## API commands

Commands are colon separated an can be send via USB/Serial

|Command|Description|
|:------|:----------|
|`info`|returns a colon separated string with all parameters (see below)|
|`version`|returns the software version|
|`on:<0/1>`|switch stirren on|
|`off:<0/1>`|switch stirren off|
|`bon:<0/1>`|switch boost mode on|
|`boff:<0/1>`|switch boost mode off|
|`speed:<0/1>:<rpm>`|set stirrer speed (FANMIN-FANMAX rpm)|
|`bspeed:<0/1>:<rpm>`|set stirrer boost speed (speed-FANMAX rpm)|
|`btime:<0/1>:<min>`|set boost time (0-60)|
|`con:<0/1>`|switch catch mode on|
|`coff:<0/1>`|switch catch mode off|
|`ctime:<0/1>:<min>`|set catch mode interval (60-240 min)|
|`rtime:<0/1>:<sec>`|set speed rise time (0-240 sec)|
|`otime:<0/1>:<hour>`|switch stirrer off after time in hours (1-99 hour, 0 deactivates switch off)|

All commands return a colon separated string with all current parameters:

|Element|Stirrer|Variable in stir.ino|Description|
|:------|:------|:-------------------|:----------|
| 0|0|`F[0]`     |state (0=on, 1=off)|
| 1|0|`v[0]`     |speed (rpm)|
| 2|0|`b[0]`     |boost speed (rpm)|
| 3|0|`rpm[0]`   |rpm|
| 4|0|`xpm[0]`   |averaged rpm|
| 5|0|`r[0]`     |regulation value|
| 6|0|`bstate[0]`|boost state (0=on, 1=off)|
| 7|0|`btime[0]` |boost time (min)|
| 8|0|`cat[0]`   |fish catch mode state (0=on, 1=off)|
| 9|0|`ctime[0]` |fish catch time interval (min)|
|10|0|`rtime[0]` |speed rise time (sec)|
|11|0|`otime[0]` |switch off time (hour)|
|12|0|           |boost remain (sec)|
|13|0|           |off timer remain (sec)|
|14|1|`F[1]`     |state (0=on, 1=off)|
|15|1|`v[1]`     |speed (rpm)|
|16|1|`b[1]`     |boost speed (rpm)|
|17|1|`rpm[1]`   |rpm|
|18|1|`xpm[1]`   |averaged rpm|
|19|1|`r[1]`     |regulation value|
|20|1|`bstate[1]`|boost state (0=on, 1=off)|
|21|1|`btime[1]` |boost time (min)|
|22|1|`cat[1]`   |fish catch mode state (0=on, 1=off)|
|23|1|`ctime[1]` |fish catch time interval (min)|
|24|1|`rtime[1]` |speed rise time (sec)|
|25|1|`otime[1]` |switch off time (hour)|
|26|1|           |boost remain (sec)|
|27|1|           |off timer remain (sec)|
|28| |           |0=ok, 1=error|
|29| |           |software version|

## Schematics

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/schematic_leonardo.png" width=500>
<img src="https://github.com/micworg/stir/blob/master/images/schematic_uno.png" width=500>
</p>
