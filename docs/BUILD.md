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

To build fw with more recent arduino core 2.x based on IDF 4.4 (still not default for Platformio as of Apr 2022)
```sh
platformio run -e espem_core202
```

### Making a LittleFS image file with web resources
To handle WebUI it is required to build a LittleFS image and upload it to the controller. The image contains files from the [EmbUI](https://github.com/vortigont/EmbUI) framework and js/css files for the ESPEM project. There is no prebuild image since it is hard to maintain it, instead there is a shell script that downloads required files from github, repacks it and places under `/data` directory. That directory is used to create and upload LittleFS image to the controller. Run
```sh
cd resources
./respack.sh
```
It should populate `/data` dir with `js`, `css`, `index.html.gz`, etc...

To upload LitlleFS image for ESP32 (until core v2 is out) it is required to use an uploader binary *mklittlefs*. Pls, download version for your OS from [here](https://github.com/earlephilhower/mklittlefs/releases) and put the binary to the root dir of the project.

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
