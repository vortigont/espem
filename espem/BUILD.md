## How to build

The project could be build with [PlatformIO](https://platformio.org/). Before building make a hardware connection to PZEM. All the details could be found [here](https://github.com/olehs/PZEM004T/wiki).

Steps to run:
 - Configuring and building firmware
 - Making a LittleFS image file with web resources
 - Uploading firmware
 - Uploading LittleFS image
 
### External Libs used to build/use the firmware
- [EmbUI](https://github.com/vortigont/EmbUI) framework is used to construct Web interface, manage WiFi, store configuration, etc
- [PZEM004T](https://github.com/olehs/PZEM004T) library to interact with PZEM power meter
- [FTPClientServer](https://github.com/charno/FTPClientServer) to access ESP's file system (optional)
- [AmCharts](https://www.amcharts.com/) JavaScript Charts & Maps to draw gauges and charts


### Configuring and Building
First, copy default config file `espem/default_config.h` to `espem/user_config.h` and change default options. There are not so many things to change there.
 - Choose either to use Hardware Serial port or SoftwareSerial pins by uncommenting `#define ESPEM_USE_HWSERIAL`. Detailed info about using HWSerial with ESP8266 could be found [here](https://github.com/olehs/PZEM004T/wiki/Connecting-PZEM-to-ESP8266).
 - Debuging might be enabled via `#define ESPEM_DEBUG Serial`
 - If using softwareSerial `#define PIN_RX` `#define PIN_TX`

To build fw for espem8266 run:
```sh
platformio run
```
To build fw for espem8266 with PZEM-004Tv30 run:
```sh
platformio run -e espem8266v30
```
To build espem8266 debug version, run:
```sh
platformio run -e espem8266debug
```
it will build fw with lot's of debug info being printer to the Hardware `Serial1` port.

To build fw for ESP32 with PZEM-004Tv30 run:
```sh
platformio run -e esp32v30
```


### Making a LittleFS image file with web resources
To handle WebUI it is required to build a LittleFS image and upload it to the controller. The image contains files from the [EmbUI](https://github.com/vortigont/EmbUI) framework and js/css files for the ESPEM project. The is no prebuild image since it is hard to maintain it, instead there is a shell script that downloads required files from github, repacks it and places under `/data` directory. Tha directory is used to create and upload LittleFS image to the controller. Run
```sh
cd resources
./respack.sh
```
It should populate `/data` dir with `js`, `css`, `index.html.gz`, etc...

Now the FS image and firmware could be uploaded to the controller
```sh
platformio run -t uploadfs
platformio run -t upload
```

That's it. Controller should reboot and enable WiFi. Look for the open Access point names like EmbUI-xxxx, connect to it and open WebUI http://192.168.4.1/, proceed with settings via WebUI.
