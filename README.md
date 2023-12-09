__[BUILD](/docs/BUILD.md) | [USAGE](/docs/USAGE.md) | [CHANGELOG](/CHANGELOG.md)__ | [![PlatformIO CI](https://github.com/vortigont/espem/actions/workflows/pio_build.yml/badge.svg)](https://github.com/vortigont/espem/actions/workflows/pio_build.yml)
## Energy monitor based on ESP32 controller and Peacefair PZEM-004Tv30 Power Meter

A code for ESP32 controller and a set of web-tools to gather, analyse, monitor and view stat and graphs
for [Peacefair](https://peacefair.aliexpress.com/store/1773456/) PZEM-004Tv30 PowerMeter.


## Features
- WebUI via self-hosted HTTP/WebSocket server
- Real-time gauges and power charts
- MQTT publishing metrics data in json format
- 3 level Tiered TimeSeries metrics collector in controller's memory pool
- data/metrics export via json
- compressed OTA updating via [esp32-flashz](https://github.com/vortigont/esp32-flashz) lib
- espem could be build to run via DummyPZEM emulator for prototyping and firmware testing without connecting to a real PZEM device

## ESPEM WebUI Dashboard

<img src="/examples/espemembui.png" alt="espem ui" width="50%"/>

## MQTT
MQTT server publishing and connection could be configured via "Settings" - "MQTT". 
It will publish pzem metrics to topic `~/pub/pzem/jmetrics` on each update cycle. Where `~` is topic prefix, default is `EmbUI/[DeviceID]/`.

Message structure is a dictionary with PZEM metrics in an integer format in a way PZEM sends it via MODBUS.
`[{"stale":false,"age":999,"U":2180,"I":503,"P":778,"W":560,"Pf":71,"freq":505}]`

I.e. <br>
`U` - denoted voltage in decivolts<br>
`I` - current im mA<br>
`P` - power in deciwatts<br>
`W` - energy in Wh<br>
`Pg` - power factor in percents<br>
`freq` - is frequency in dHz<br>
`stale` - denotes if data is stale, i.e. has not been updated recently<br>
`age` - data age in ms<br>

<img src="/examples/mqtt.png" alt="espem mqtt" width="75%"/>

## Tiered TimeSeries data Sampling
Controller will keep a history of previous data received from PZEM in it's memory in a tiered memory pool. It is commonly used for time series data where the longer the data age then less frequent is sampling rate of the data to keep. All data resides in volatale `memory`, so it will be lost on power cycle or reset. 
By default there are 3 levels of TimeSeries in a pool



| Level | Samples count | Sampling interval | Time range          |
| ----- | ------------- | ----------------- | ------------------- |
| L1    | 900           | 1 sec             | 15 min              |
| L2    | 1000          | 15 sec            | 250 min (abt 4 hrs) |
| L3    | 1000          | 300 sec           | ~83 hrs            |

Number of samples and interval could be adjusted per each level via "Espem setup" - "TimeSeries collector" configuration.

#### ESPEM TS Options

<img src="/examples/ts_setup.png" alt="espem ts opts" width="75%"/>



#### Averaging
For levels with sampling interval more that 1 second an averaging function is used to calculate mean value between intervals. I.e. if sampling interval is set to 60 sec and you power level was about 5 watt for 55 seconds and became 100 watts for the last 5 sec when sampling was taken, then resulting power value would be slightly over 5 watts, but not 100 W.

#### Memory consumption
1000 samples takes about 25 KiB of RAM memory (one sample takes 28 bytes), so plan you pool accordingly. If your board has SPI-RAM then you can have a huge pool worth monthes of data to be kept :)

#### Data export
TimeSeries Data could be exported in json format per each tier level

Tier 1 URL - [http://espem/samples.json?tsid=1](http://espem/samples.json?tsid=1)<br>
Tier 2 URL - [http://espem/samples.json?tsid=2](http://espem/samples.json?tsid=2)<br>
Tier 3 URL - [http://espem/samples.json?tsid=3](http://espem/samples.json?tsid=3)<br>

An example of exported data:
```
[
    {"t":1701876803000,"U":216.60,"I":0.51,"P":88,"W":7,"hz":49.7,"pF":0.79},
    {"t":1701877103000,"U":225.00,"I":0.52,"P":90,"W":14,"hz":50.1,"pF":0.77}
]
```
where:<br>
`t` - is a unix timestamp in milliseconds (prefered for js processing)
other keys are PZEM metrics in float format


## Legacy v2.x version
An older ESPEM version 2 was based on 3rd party lib. It's code still available under [2.x branch](https://github.com/vortigont/espem/tree/v2).
ESPEM Ver 3 switched to it's own library [pzem-edl](https://github.com/vortigont/pzem-edl). I wrote this lib to overcome limitations of the classic [olehs](https://github.com/olehs/PZEM004T)'s and [mandulaj](https://github.com/mandulaj/PZEM-004T-v30)'s libs. Being versatile those libs provided only basic functions talking to PZEM's using Arduino's blocking IO via serial port. New lib uses event-driven approach and provides extendable design API for multiple PZEM communication over single port.

### Feature comparison
|                      | [Version 2.x](https://github.com/vortigont/espem/tree/v2) | Version 3.0       |
| -------------------- | --------------------------------------------------------- | ----------------- |
| ESP8266              | YES                                                       | NO                |
| ESP32                | YES                                                       | YES               |
| PZEM004v30           | YES                                                       | YES               |
| old PZEMv30          | YES                                                       | NO       |
| Time Series Charts   | YES (Basic)                                               | YES (extendable)  |
| TS in PSRAM          | NO                                                        | YES               |



### Additional tools
Under /www there is a set of php/sql scripts that could be hosted undel LAMP to gather and calculate stats over long-term periods. Little bit outdated but still usable.
No need for any cloud services, spyware etc... just a raspberry/orangepi running web-server with sqlite/mysql DB. It's possible to collect data from any number of PZEM monitors and store it in the DB for a long-term stats or get a PowerChart sampled data from the espem itself.

_*An example of exernal daily stats dashboard*_

![espem gauges](/examples/webstat/gauges.th.png)


### Power chart with sampled data

![espem sampled data](/examples/webstat/sampleschart01.th.png)



#### License

This code is available under the GPLv3 license
