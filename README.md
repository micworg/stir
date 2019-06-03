# Dual magnetic stir controller

## Overview

This is the Arduino sketch to build a double magnetic stirrer based on 2 fans, a 1602 LCD display, a KY-040 encoder and an Arduino (e.g. Uno, Leonardo, Pro Micro).

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/stir.jpg" width=500>
</p>

In order to put the stirrer into operation you have to change the settings in stir.ino according to your setup.

## Reference

This project was created based on an idea from the Hobbybrauer-Forum and would not be possible without the ideas from there.

The forum: https://hobbybrauer.de/

The thread: https://hobbybrauer.de/forum/viewtopic.php?f=21&t=1456

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
  * **RNVAL**: random value range (a random values in this range will be added to speed) 
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
|`BINC`|boost time increment (min)|
|`CINC`|catch time increment (min)|
|`RINC`|rise time increment (sec)|
|`OINC`|off timer increment (hour)|
|`CATCHSTOP`|stop interval for stir fish catch function (ms)|
|`PWM0`, `PWM1`|PWM output pins for fan speed|
|`PWM2`|PWM output pin for LCD brightness control|
|`I0`, `I1`|interrupts for rpm measurement (2 and 3 for Leonardo and ProMicro / 0 and 1 for Uno)|
|`CLK`, `DT`, `SW`|pins for KY-040 encoder| 
|`OFF0`, `OFF1`|these pins will be set if the fan is off| 
|`OFFSTATE`|off state (LOW/HIGH)| 
|`RINTERVAL`|regulation interval (ms)|
|`RDELAY`|extra regulation delay when value changes (ms)|
|`RTOL`|regulation tolerance (rpm)|
|`RNDINTERVL`|randon value range change interval (ms)|
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
|`rnval:<0/1>:<rpm>`|set randon value range (0-1000 rpm)|
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
|12|0|`rnval[0]` |random value range (rpm)|
|13|0|`rnd[0]`   |current random value (rpm)|
|14|0|           |boost remain (sec)|
|15|0|           |off timer remain (sec)|
|16|1|`F[1]`     |state (0=on, 1=off)|
|17|1|`v[1]`     |speed (rpm)|
|18|1|`b[1]`     |boost speed (rpm)|
|19|1|`rpm[1]`   |rpm|
|20|1|`xpm[1]`   |averaged rpm|
|21|1|`r[1]`     |regulation value|
|22|1|`bstate[1]`|boost state (0=on, 1=off)|
|23|1|`btime[1]` |boost time (min)|
|24|1|`cat[1]`   |fish catch mode state (0=on, 1=off)|
|25|1|`ctime[1]` |fish catch time interval (min)|
|26|1|`rtime[1]` |speed rise time (sec)|
|27|1|`otime[1]` |switch off time (hour)|
|28|1|`rnval[1]` |random value range (rpm)|
|29|1|`rnd[1]`   |current random value (rpm)|
|30|1|           |boost remain (sec)|
|31|1|           |off timer remain (sec)|
|32| |           |software version|
|33| |           |uptime (ms)|
|34| |           |0=ok, 1=error|

## Schematics

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/schematic_leonardo.png" width=500>
<img src="https://github.com/micworg/stir/blob/master/images/schematic_uno.png" width=500>
</p>
