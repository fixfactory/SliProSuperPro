# SliProSuperPro: A Shift Light Indicator Controller
*Copyright 2023 Fixfactory*


## Overview

SliProSuperPro is a command line application that controls a [Leo Bodnar Electronics SLI-Pro device](https://www.leobodnar.com/shop/index.php?main_page=product_info&products_id=185). SliProSuperPro reads the telemetry data from a sim-racing game and displays the gear, RPM, and speed on the SLI-Pro device.

SliProSuperPro currently supports one game, [Richard Burns Rally with NGP7](https://rallysimfans.hu/rbr/index.php), but is architected to support additional games via plugins.

<p align="center">
  <img src="Docs/Images/SLI-Pro-Preview.gif" width="300" height="225" />
</p>


## How To Get It

Windows builds are available via the [releases page](https://github.com/fixfactory/SliProSuperPro/releases).


## How to Use

Simply extract the zip content into a folder and run `SliProSuperPro.exe`. The application will automatically detect when a supported game is running, read the telemetry and display the data on the SLI-Pro device.

Use `SliProSuperPro.exe --help` for a list of options.

<p align="center">
  <img src="Docs/Images/SliProSuperPro-Screenshot.png" width="412" height="239" />
</p>


## Build Instructions

The preferred method for building is with Microsoft Visual Studio 2022. Open the solution `SliProSuperPro.sln` and build.