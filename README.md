# SakuraIo Evaluation Board Standard

## Overview
This program is example code of [sakura.io Evalaution board](https://os.mbed.com/platforms/SAKURAIO_EVB_01/).

### Functions

- Periodic measure from onboard sensors(period is 200ms)
  - Motion sensor(gyro, accelometer, magnetometer)
  - Environment sensor(temperatur, humidity, airpressur)
  - GPS(longitude, latitude, timestamp)
- Periodic send the measuring datas to sakura.io platform(period is 300sec)
- Output the measured datas output to USB-Serial port
  - baudrate is 9600bps
- Can select on / off of periodic running with switch `SW5`

## Description
See the [Getting Started](https://github.com/sakuraio/SakuraIO_Evaluation_Board_Standard/wiki/GettingStarted) page.
