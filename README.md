__[BUILD](/docs/BUILD.md) | [USAGE](/docs/USAGE.md) | [CHANGELOG](/CHANGELOG.md)__ | [![PlatformIO CI](https://github.com/vortigont/espem/actions/workflows/pio_build.yml/badge.svg)](https://github.com/vortigont/espem/actions/workflows/pio_build.yml)
## Energy monitor based on ESP32 controller and Peacefair PZEM-004Tv30 Power Meter

A code for ESP32 controller and a set of web-tools to gather, analyse, monitor and view stat and graphs
for [Peacefair](https://peacefair.aliexpress.com/store/1773456/) PZEM-004Tv30 PowerMeter.


## Features
- WebUI via self-hosted HTTP/WebSocket server
- Real-time gauges and power charts
- metrics collector in controller's memory pool
- data/metrics export via json


## Legacy v2.x version
An older ESPEM version 2 was based on 3rd party lib. It's code still available under [2.x branch](https://github.com/vortigont/espem/tree/v2).
ESPEM Ver 3 switched to it's own library [pzem-edl](https://github.com/vortigont/pzem-edl). I wrote this lib to overcome limitations of the classic [olehs](https://github.com/olehs/PZEM004T)'s and [mandulaj](https://github.com/mandulaj/PZEM-004T-v30)'s libs. Being versatile those libs provided only basic functions talking to PZEM's using Arduino's blocking IO via serial port. New lib uses event-driven approach and provides extendable design API for multiple PZEM communication over single port.

### Feature comparison
|                |[Version 2.x](https://github.com/vortigont/espem/tree/v2)  |Version 3.0|
|----------------|--------------|---------------|
|ESP8266         |YES           |NO             |
|ESP32           |YES           |YES            |
|PZEM004v30      |YES           |YES            |
|old PZEMv30     |YES           |NO (planned)   |
|PZEM017 (DC version)   |NO|NO (planned)|
|3 phase option         |NO|YES (in progress)|
|Time Series Charts     |YES (Basic)|YES (extendable)|
|TS in PSRAM         |NO|YES|



### ESPEM WebUI Dashboard

<img src="/examples/espemembui.png" alt="espem ui" width="50%"/>

### ESPEM WebUI Options

<img src="/examples/espemembui_setup.png" alt="espem opts" width="50%"/>

### Additional tools
Under /www there is a set of php/sql scripts that could be hosted undel LAMP to gather and calculate stats over long-term periods. Little bit outdated but still usable.
No need for any cloud services, spyware etc... just a raspberry/orangepi running web-server with sqlite/mysql DB. It's possible to collect data from any number of PZEM monitors and store it in the DB for a long-term stats or get a PowerChart sampled data from the espem itself.

_*An example of exernal daily stats dashboard*_

![espem gauges](/examples/webstat/gauges.th.png)


### Power chart with sampled data

![espem sampled data](/examples/webstat/sampleschart01.th.png)



#### License

This code is available under the GPLv3 license
