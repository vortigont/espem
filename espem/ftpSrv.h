#include <AsyncTCP.h>

#ifdef ESP_ARDUINO_VERSION
// LittleFS is embedded into arduino core >=2.0.0
#include <LittleFS.h>
#else
// LittleFS as external lib for arduino core <2.0.0
#include <LITTLEFS.h>
#define LittleFS LITTLEFS
#endif

#define FORMAT_LITTLEFS_IF_FAILED true

#ifndef FTP_USER
 #define FTP_USER "ftp"
#endif
#ifndef FTP_PASSWORD
 #define FTP_PASSWORD "ftp"
#endif

//#define FTP_DEBUG
#include <FTPServer.h>

FTPServer ftpSrv(LittleFS); // construct with LittleFS

void ftp_setup(void){
  /////FTP Setup, ensure LittleFS is started before ftp;  /////////
  if (LittleFS.begin()) {
    ftpSrv.begin(F(FTP_USER), F(FTP_PASSWORD)); //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
  }
}

void ftp_loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
}
