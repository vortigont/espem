## How to build

The project could be build with [PlatformIO](https://platformio.org/). Before building make a hardware connection to PZEM. All the details could be found [here](https://github.com/olehs/PZEM004T/wiki).

Steps to run:
 - Configuring and building firmware
 - Making a LittleFS image file with web resources
 - Uploading firmware
 - Uploading LittleFS image
 
### External Libs used to build/use the firmware
- [EmbUI](https://github.com/vortigont/EmbUI) framework is used to construct Web interface, manage WiFi, store configuration, etc
- [pzem-edl](https://github.com/vortigont/pzem-edl) event-driven library to interact with PZEM power meter
- [FTPClientServer](https://github.com/charno/FTPClientServer) to access ESP's file system (optional)
- [AmCharts](https://www.amcharts.com/) JavaScript Charts & Maps to draw gauges and charts


### Configuring and Building
Most of the setup is done via WebUI. Once fw is flashed and FS image uploaded it is required to connect to the WiFI HotSpot and set WiFi options, TimeZone, and PZEM serial port/pins settings.
Two platformio env's are provided to build:

To build regular fw for esp32, just run:
```sh
platformio run
```
To build fw with debugging enabled, run:
```sh
platformio run -e espem_debug
```
Remember, you can't use serial0 for PZEM comm if build debug version. Default uart port is port_1. Any available esp32 pins could be mapped to port_1

To build fw with an older arduino core 1.5.x based on IDF 3.x
```sh
platformio run -e espem_pio350
```

### Making a LittleFS image file with web resources
To handle WebUI it is required to build a LittleFS image and upload it to the controller. The image contains files from the [EmbUI](https://github.com/vortigont/EmbUI) framework and js/css files for the ESPEM project. There is no prebuild image since it is hard to maintain it, instead there is a shell script that downloads required files from github, repacks it and places under `/data` directory. That directory is used to create and upload LittleFS image to the controller. Run
```sh
cd resources
./respack.sh
```
It should populate `/data` dir with `js`, `css`, `index.html.gz`, etc...


OBSOLETE: <del>To upload LitlleFS image for ESP32 (until core v2 is out) it is required to use an uploader binary *mklittlefs*. Pls, download version for your OS from [here](https://github.com/earlephilhower/mklittlefs/releases) and put the binary to the root dir of the project.</del>

Now the FS image and firmware could be uploaded to the controller, just run
```sh
platformio run -t upload
platformio run -t uploadfs
```

or debug version
```sh
platformio run -e espem_debug -t upload
platformio run -e espem_debug -t uploadfs
```



That's it. Controller should reboot and enable WiFi. Look for the open Access point names like EmbUI-xxxx, connect to it and open WebUI http://192.168.4.1/, proceed with settings via WebUI. Check [USGE](USAGE.md) page for more details.


### Copressed OTA updates
Uploading updated firmware could be done either via WebUI or curl CLI tool. It is possible to compress images with zlib to reduce upload size, decompression is handled via [esp32-flashz](https://github.com/vortigont/esp32-flashz) lib.

OTA could be intergated to Platformio's scritping, check supplied platformio.ini for OTA template. it need two options:
 - OTA_url - upload url to access espem
 - OTA_compress - use zlib compression on upload


uploading fw with curl CLI

for firmware image
```
curl -v http://$ESPHOST/update -F "img=fw" -F 'name=@.pio/build/espem/firmware.bin.zz'
```

for fs image
```
curl -v http://$ESPHOST/update -F "img=fs" -F 'name=@.pio/build/espem/littlefs.bin.zz'
```

to compress and upload image on-the-fly with [pigz](https://zlib.net/pigz/) tool
```
pigz -9kzc .pio/build/espem/firmware.bin | curl -v http://$ESPHOST/update -F "img=fw" -F file=@-
pigz -9kzc .pio/build/espem/littlefs.bin | curl -v http://$ESPHOST/update -F "img=fs" -F file=@-
