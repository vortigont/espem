# Change Log

Known issues:
 - project may not build on platformio due to broken manifest file. WA: use stage version of arduinoespressif8266
   or fix 'platform' in manifest file for EspSoftwareserial lib


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