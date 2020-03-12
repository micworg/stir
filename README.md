# Dual magnetic stir controller V2


## Overview

This is the Arduino project to build a double magnetic stirrer based on 2 fans, a 1602 LCD display, a KY-040 encoder and an Arduino Nano and a special PCB designed in the Hobbybrauer-Forum.

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/stir+pcb.jpg" width=700>
</p>

V2 supports dual voltage for the fans which increases the speed range.

In order to put the stirrer into operation you have to change the settings in stir.ino according to your setup (see below).

The stir V2 software is compatible with V1 hardware if you have used neither the RS232 Api nor the OFF0 and OFF1 relais pins.


## Part List

* 1x PCB "Stir V2.0"
* 1x Arduino Nano V3
* 1x [Voltage regulator TE818 5-24V to 5V 3A step down buck converter](https://www.ebay.de/itm/5x-Voltage-Regulator-5-24V-to-5V-3A-Step-Down-Buck-Converter-Power-Module-TE818/133109140535?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649)
* 2x [Relais Omron G5Q-1-EU 5DC switching printrelais 5 V/DC 5A](https://www.conrad.de/de/p/omron-g5q-1-eu-5dc-printrelais-5-v-dc-5-a-1-wechsler-1-st-503930.html)
* 2x [Diode 1N4148 Diotec](https://www.conrad.de/de/p/stmicroelectronics-schottky-diode-gleichrichter-1n5822-do-201ad-40-v-einzeln-168275.html)
* 1x [Electrolytic capacitor 1000µF/16V Yageo SE016M1000B5S-1015](https://www.conrad.de/de/p/yageo-se016m1000b5s-1015-elektrolyt-kondensator-radial-bedrahtet-5-mm-1000-f-16-v-20-x-h-10-mm-x-15-mm-1-st-445386.html)
* 2x [Diode 1N5822 STMicroelectronics Schottky-Diode](https://www.conrad.de/de/p/diotec-ultraschnelle-si-diode-1n4148-sod-27-75-v-150-ma-162280.html)
* 4x [Metal film resistor 10 KOhm 0,6 W](https://www.conrad.de/de/p/rs-pro-metallschichtwiderstand-10k-0-5-0-6w-807200469.html)
* 2x Fan BeQuiet SilentWings 3 120mm PWM Highspeed or Noctua NF-R8 Redux-1800 80mm PWM
* 1x Power supply 12V-16V DC / 2A
* 1x Built-in socket for plug-in power supply unit
* 1x I2C 16x2 Arduino LCD Display Module
* 1x [Rotary encoder KY-040](https://www.conrad.de/de/p/joy-it-kodierter-drehschalter-rotary-encoder-1695709.html)
* 2x Magnet mount (3D printed)
* 4x Neodymium magnet 20x10 mm (N52, 1 oder 2 mm dick)

In addition, a housing is required that can be individually designed according to possibilities and capabilities.

<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/v2_board.jpg" width=350>
</p>


## Reference

This project was created based on an idea from the german Hobbybrauer-Forum and would not be possible without the ideas from there.

The forum: https://hobbybrauer.de/

The thread: https://hobbybrauer.de/forum/viewtopic.php?f=21&t=1456

Special thanks for development and testing:

* Herbert Schmid 
* Adrian Sigel
* Jens Warkentin
* Bastian Werner


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
  * **CFG**: Configuration menu 

Functions in the congirutation sub menu
* **Short Press**: switch between config option and and value.
* **Long press**: exit configuration menu.
* **Turn**: select configuration option or change value.


## Display Indicators:

* **Bottom Left**: Menu
* **Bottom Center/Right**: displays the set values for the stirrers depending on the selected menu item.
* **Top Center/Right**: displays the current stirrer speed (or 'OFF' or 'CAT' (Fishing)).
* **Top Left**: shows the remaining time of the boost or switch off function.

## Settings (in stir.ino)

|Value|Config|Description|
|:----------------|:-----|:----------|
|`LGHT`           |`LGHT`|LCD display brightness|
|`SPEEDINC1`      |`SPI1`|increment of speed values when turning the encoder (1. range)|
|`SPEEDINC2`      |`SPI2`|increment of speed values when turning the encoder (2. range)|
|`SPEEDINC3`      |`SPI3`|increment of speed values when turning the encoder (3. range)|
|`SPEEDSTEP1`     |`SPS1`|speed increment steps (increment changes at these values, separates range 1 and 2)|
|`SPEEDSTEP2`     |`SPS2`|speed increment steps (increment changes at these values, separates range 2 and 3))|
|`FANMIN`         |`FMIN`|fan minimum speed (should be a value at which the fan runs safely) (rpm)|
|`FANMAX`         |`FMAX`|fan maximum speed  (it's important that this is the real maximum value of the fan) (rpm)|
|`FANINIT`        |`FINI`|set fans to high voltage for 3 sec at boot (0/1)|
|`BOFF`           |`BOFF`|if set boost off will also turn the normal mode off (0/1)|
|`BINC`           |`BINC`|boost time increment (min)|
|`CINC`           |`CINC`|catch time increment (min)|
|`RINC`           |`RINC`|rise time increment (sec)|
|`OINC`           |`OINC`|off timer increment (hour)|
|`RNDINC`         |`RINC`|randon value increment (rpm)|
|`CATCHSTOP`      |`CSTP`|stop interval for stir fish catch function (ms)|
|`PWM0`, `PWM1`   |      |PWM output pins for fan speed|
|`PWM2`           |      |PWM output pin for LCD brightness control|
|`I0`, `I1`       |      |interrupts for rpm measurement (0/1 for Uno/Nano, 2/3 for Leonardo/ProMicro)|
|`SWAPENC`        |`SWEN`|switch KY-040 encoder direxction (0/1)| 
|`CLK`, `DT`, `SW`|      |pins for KY-040 encoder| 
|`R0`, `R0`       |      |voltage select relais pin| 
|`RTHRES`         |`RTHR`|voltage switch threshold (rpm)| 
|`RINTERVAL`      |`XINT`|regulation interval (ms)|
|`RDELAY`         |`XDEL`|extra regulation delay when value changes (ms)|
|`RTOL`           |`XTOL`|regulation tolerance (rpm)|
|`RNDINTERVL`     |`RINT`|randon value range change interval (ms)|
|`SINTERVAL`      |`SINT`|speed measurement interval (ms)|
|`SAVERAGE`       |`SAVG`|speed measurement average|
|`SAVETAG`        |      |parameter save identifier|
|`SAVEDELAY`      |      |delay in seconds before parameter will be saved to EEPROM|
|`LCDB`           |      |LCD brightness steps (10 values, 0=off, 255=max)|
|`RESET`          |`RSET`|change this value in configuration menu to 1 for factory reset|

