# Change Log

* adopted building with Arduino core >2.0.0
* UI now can detect if PZEM is disconnected or unreachable and show "err" values
+ energy offset feature

## v 3.0.0 (2022-01-24)
 * bump EmbUI version up to v2.6
 * switch to pzem-edl lib, version 2.x becomes legacy
     - drop support for older PZEM004 model
     - drop support for esp8266
 * UART configuration via WebUI
 * 1 level TimeSeries chart only (for now)

## v 2.0.1 (2021-09-03)
+ WebUI displays for Voltage, Current, Energy 
+ ESP32 support (hardware serial only, with pin remapping)
+ PZEM-004Tv30 support (WIP)
- SoftwareSerial from now on is deprecated
+ heap curruption fix
* bump EmbUI version up to v2.5

## v 2.0.0 (2021-04-12)

  Completely reworked project.
    - UI switched to [EmbUI](https://github.com/vortigont/EmbUI) framework
    - embeded dashboard using websockets for real-time data/gauges updates
    - most of the build-time defines moved to WebUI controls
    

## v 1.1.x Obsolete

Added:
    poller: merge with www
      - poller script now resides along with other www scripts
      - Merged poller's script config with www's config.php

    FW: add ESP Core version output to http /ver responce

Fixed:
 - Current month stat calculation fixed for more than 1 year stat

Changed:
    platformio:
     - pio environments for different ESP core's (stable/stage/core_2.3.x)
     - build options reducing FW size
        - add script to strip newlib floats and reduce fw size
          (https://github.com/arendst/Sonoff-Tasmota/commit/f60c570eaccc3b0f993bf220c58f54b39e1f0419)
        - add flags to remove DEBUG/GLOBAL_HWSERIAL for stable env

Known issues:


## v 1.1.0 (2017-07-19)

Added:
 - FW: http://espem/ver now returns json-formatted data with ChipID, Flash Size, firmware SDK version, firmware version, CPU clock speed, free heap size, uptime in seconds
 - Statpage: Added dropdown selector with all available power-meters in DB
 - Samples chart: retrieve meter hostname from DB via request arg ?devid=<meterid>
 - FW: config Tunable to swap hw_serial pins

Fixed:
 - Added woraround when sometimes meter returns all zero values (zero Energy counnter poisons DB stat)

Other:
 - Switched to a forked version of PZEM004T lib untill fix for esp is merged to oleh's master
 - FW doesn't build under platformio 'because of the wrong manifest for EspSoft


## 1.0.2 (2017-04-21)

Added:

 - /ota now accepts url arg to specify custom update url (e.x. curl -G -v http://espem/ota --data-urlencode 'url=http://upd.server.org/esp/espem.bin')
 - Added OTA url customize via config form

Fixed:
 - fixed bug with json sprintf
 - fixed bug in eeprom config update