## Energy monitor based on ESP8266 controller and Peacefair PZEM-004T Power Meter

A code for ESP8266 controller and a set of web-tools to gather, analyse, monitor and view stat and graphs
for Peacefair PZEM-004T PowerMeter.

## Features
- WebUI via self-hosted HTTP/WebSocket server
- Real-time gauges and power charts
- metrics collector in memory pool
- data/metrics export via json 

### WebUI Dashboard

![espem ui](https://github.com/vortigont/espem/raw/master/examples/webstat/espemembui.png)

### WebUI Options

![espem opts](https://github.com/vortigont/espem/raw/master/examples/webstat/espemembui_setup.png)


### Additional tools
A set of php/sql scripts to gather and calculate stats over long periods. No need for cloud services, just a raspberry/orangepy running web-server with sqlite/mysql DB. It's possible to collect data from any number of PZEM monitors and store it in the DB for a long-term stats or get a PowerChart sampled data from the espem itself.

### An example of daily stat dashboard

![espem gauges](https://github.com/vortigont/espem/raw/master/examples/webstat/gauges.th.png)


### Power chart with sampled data

![espem sampled data](https://github.com/vortigont/espem/raw/master/examples/webstat/sampleschart01.th.png)


### TODO:
Write docs


#### License

This code is available under the GPLv3 license
