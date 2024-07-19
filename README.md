# SliProSuperPro: A Shift Light Indicator Controller
*Copyright 2023, 2024 Fixfactory*


## Overview

SliProSuperPro is a command line application that controls a [Leo Bodnar Electronics SLI-Pro device](https://www.leobodnar.com/shop/index.php?main_page=product_info&products_id=185). SliProSuperPro reads the telemetry data from a sim-racing game and displays the gear, RPM, and speed on the SLI-Pro device.

<p align="center">
  <img src="Docs/Images/SLI-Pro-Preview.gif" width="300" height="225" />
</p>

SliProSuperPro currently supports my three favorite games:
- [Richard Burns Rally](https://rallysimfans.hu/rbr/index.php)
- [iRacing](https://www.iracing.com)
- [American Truck Simulator](https://americantrucksimulator.com)

Support for additional games can be added via plugins.


## How To Get It

Windows builds are available via the [releases page](https://github.com/fixfactory/SliProSuperPro/releases).


## How to Use

Simply extract the zip content into a folder and run `SliProSuperPro.exe`. The application will automatically detect when a supported game is running, read the telemetry and display the data on the SLI-Pro device.

Use `SliProSuperPro.exe --help` for a list of options.

<p align="center">
  <img src="Docs/Images/SliProSuperPro-Screenshot.png" width="412" height="239" />
</p>


## Richard Burns Rally Configuration

UDP Telemetry must be manually turned ON in your Richard Burns Rally installation for SliProSuperPro to be able to read the telemetry. In the RSF Launcher, go to *Advanced Settings* > *Telemetry* and turn ON UDP Telemetry. The address must be `127.0.0.1:6776`. If you also want to send the telemetry to another application such as Sim Hub, add another address (e.g. `127.0.0.1:6777`) and configure the app to use that address.


## iRacing Configuration

No configuration is required in iRacing; by default the game outputs live telemetry via a shared memory file which is read by SliProSuperPro.

The optional `iRacing.Overrides.json` file can be used to specify RPM overrides for some cars. Because the default RPM values reported by the iRacing SDK are wrong for most cars, it is recommended that you test each car and fill in the correct RPM values.

The plugin supports specifying global RPM values that are the same for every gear, or specifying different RPM values for each gear. Use existing overrides as an example. `firstRPM` is the RPM at which the first LED turns On, and `lastRPM` is the RPM at which the last LED turns On causing the shift lights to blink.


## American Truck Simulator Configuration

The plugin `SPSP.ATS.Plugin.dll` must be copied in your American Truck Simulator plugin folder which is typically found at `C:\Program Files (x86)\Steam\steamapps\common\American Truck Simulator\bin\win_x64\plugins`. The .dll to copy can be found in the `\Game Plugins\American Truck Simulator` folder in the zip file. Then when launching American Truck Simulator, the game will warn you about advanced SDK features being used. Simply ckick OK.


## Build Instructions

The preferred method for building SliProSuperPro from source is with Microsoft Visual Studio 2022. Open the solution `SliProSuperPro.sln` and build. Generate a .zip package with `py Package.py`.
