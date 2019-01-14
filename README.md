## Energy monitor based on ESP8266 controller and Peacefair PZEM-004T Power Meter

A code for ESP8266 controller and a set of web-tools to gather, analyse, monitor and view stat and graphs
for Peacefair PZEM-004T PowerMeter.
No need for cloud services, just a raspberry/orangepy running web-server with sqlite/mysql DB. It's possible to collect data from any number of PZEM monitors and store it in the DB for a long-term stats or get a PowerChart sampled data from the espem itself.

### An example of daily stat dashboard

![espem gauges](https://github.com/vortigont/espem/raw/master/examples/webstat/gauges.th.png)


### Power chart with sampled data

![espem sampled data](https://github.com/vortigont/espem/raw/master/examples/webstat/sampleschart01.th.png)

### Monthly stats table
![espem stats table](https://github.com/vortigont/espem/raw/master/examples/webstat/stat_table2.png)

### TODO:
Write docs


#### License

This code is available under the MIT license