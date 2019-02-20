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
* **Long press when menu is selected**: Lock all functions (unlock also by long press)
* **When a stirrer is selected**:
  * **SPEED**: turn sets speed, long press switches stirrer on or off.
  * **BOOST**: turn sets boost speed, long press activates/deactivates the boost function (running time is shown in the display).
  * **BTIME**: turn sets the boost time in minutes.
  * **CATCH**: turn activates/deactivates the fishing function
  * **CTIME**: turn sets the interwall in minutes for the fish catching function

## Display Indicators:

* **Bottom Left**: Menu
* **Bottom centre/right**: Displays the set values for the stirrers depending on the selected menu item.
* **Top Center/Right**: Displays the current stirrer speed (or 'OFF' or 'CAT' (Fishing)).
* **Top left**: shows the remaining time of the boost function.


## Schematics


<p align="center">
<img src="https://github.com/micworg/stir/blob/master/images/schematic_leonardo.png" width=500>
<img src="https://github.com/micworg/stir/blob/master/images/schematic_promicro.png" width=500>
<img src="https://github.com/micworg/stir/blob/master/images/schematic_uno.png" width=500>
</p>