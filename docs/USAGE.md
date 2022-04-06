## Running

Once [uploaded](BUILD.md) fw and FS image the controller should bring up WiFi hotspot with a name like 'EmbUI-xxxxxx'. Connect to the hotspot and open WebUI either by accessing URL http://192.168.4.1/ or using any mDNS enabled app/service, like Bonjour, [mobile app](https://play.google.com/store/apps/details?id=com.druk.servicebrowser), Windows Network Neighborhood, etc... It should auto-open WiFi setup.


<img src="/examples/embui_setup.jpg" alt="EmbUI Setup" width="30%"/>

Open WiFi Client setup, enter desired hostname, your WiFi SSID and password and press "Connect". ESPEM will reconnect to your WiFi and disable internal HotSpot.

<img src="/examples/embui_wifi.jpg" alt="EmbUI WiFi" width="30%"/>

Once it connects to your WiFi, open `ESPEM Setup` page

<img src="/examples/espemembui_setup.png" alt="ESPEM Setup" width="30%"/>

### ESPEM Options

##### Power Factor fix
After rapid changes in power/current flow PZEM sometimes returns weird values. This option adjusts the current so that Power Factor won't exceed 1.

##### Meter Polling
Enabled periodic PZEM polling for all metrics possible, dashboard needs this to update gauges.

##### Poll rate
Polling interval in seconds, applied immidiately

##### Metrics collector
Metrics collector is a pool in controller's RAM that works as a circular buffer storing polled values. Once the buffer is full an older values are overwritten with a more recent ones. Dashboard power serial chart depends on this.

##### Metrics RAM pool size
Size of RAM pool allocated for circular buffer (KiB). Beware of setting too large pool, controller could crash due to memory shortage. 8-16 KiB is a good value for 8266. ESP32 could do much better here.
Pool could be resized in real time. If pool is shrinked than some older stored data is discarded. There is no guarantee that most recent data would be kept.
If pool is enlarged than full reallocation is done, no previous data preserved.

##### Metrics collector status
`Running` - mem pool allocated and polled values are being writing to the buffer

`Paused` - mem pool allocated but polled values are not saved in buffer

`Disabled` - No memory pool allocated, only the last polled value is kept in memory

### HTTP API
`http://espem/getdata` - get current metrics (JSON format)

`http://espem/samples.json` - get time-series data from in RAM circular buffer (JSON format)

`http://espem/fw` - get firmware version info and memory stat (JSON format)