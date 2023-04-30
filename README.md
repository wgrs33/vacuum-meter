# About
This project was based on a device done by Denis Tauvel ([LINK](http://denis.tauvel.free.fr/twinmax_maison.htm)). I improved the code, leaving the menu design almost the same.

## Install requirements
* Arduino IDE >= 1.8
* CMake >= 3.20

## Parts
* Arduino nano with ATmega168P
* Encoder with a button (or a separate button)
* Two pressure sensors MPX6115AC6U
* 2x16 LCD with I2C communication ([I2C adapter board](https://www.amazon.com/LCD1602-Display-I2C-Interface-Module/dp/B07D3YPK3H))

## Connections
* A0 - pressure sensor
* A1 - pressure sensor
* A4 - I2C SDA
* A5 - I2C SCL
* D2 - encoder A
* D3 - encoder B
* D8 - encoder button

## Basic funtions

### Synchronization
The LCD show two input values in mbar. If both values are the same, then the graph bar is set to 50%, otherwise it will be greater or smaller.

### Vacuum
**Only first input is used here.**

The visible value is a difference between the first input value and the atmospheric pressure value (stored during calibration process).

### Absolute pressure
**Only first input is used here.**

Lcd shows first input value in mbar and mmHg.

### Calibrate
Calibration is done automatically when the device starts. By using this function you will override the previously stored values.
The atmospheric pressure and a delta between inputs are stored in the memory. Delta is used to compensate any differences between the sensor readings for the same conditions.

# Commands

## Configure

```
mkdir _build
cd _build/
cmake ..
```

## Build

```
cd _build
cmake --build .
```

## Flash

```
cd _build
make VacuumMeter_flash
```
