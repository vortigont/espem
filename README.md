## Energy monitor based on ESP8266/ESP32 controller and Peacefair PZEM-004T/PZEM-004Tv30 Power Meter

A code for ESP8266/ESP32 controller and a set of web-tools to gather, analyse, monitor and view stat and graphs
for [Peacefair](https://peacefair.aliexpress.com/store/1773456/) PZEM-004T/PZEM-004Tv30 PowerMeter.

__[BUILD](/espem/BUILD.md) | [USAGE](/espem/USAGE.md) | [CHANGELOG](/CHANGELOG.md)__

## Features
- WebUI via self-hosted HTTP/WebSocket server
- Real-time gauges and power charts
- metrics collector in controller's memory pool
- data/metrics export via json


### ESPEM WebUI Dashboard

<img src="/examples/espemembui.png" alt="espem ui" width="50%"/>

### ESPEM WebUI Options

<img src="/examples/espemembui_setup.png" alt="espem opts" width="50%"/>

### Additional tools
Under /www there is a set of php/sql scripts that could be hosted undel LAMP to gather and calculate stats over long-term periods. Little bit outdated but still usable.
No need for any cloud services, spyware etc... just a raspberry/orangepi running web-server with sqlite/mysql DB. It's possible to collect data from any number of PZEM monitors and store it in the DB for a long-term stats or get a PowerChart sampled data from the espem itself.

### An example of exernal daily stats dashboard

![espem gauges](/examples/webstat/gauges.th.png)


### Power chart with sampled data

![espem sampled data](/examples/webstat/sampleschart01.th.png)


### TODO:
Write docs


#### License

This code is available under the GPLv3 license
