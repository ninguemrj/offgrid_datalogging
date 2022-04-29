/**************************************************************************************
 * SDManager_inverter.cpp (v0.1)
 * Library to be used (or not) with the PV_inverter library.
 * Created to store read data in SD card for backup purposes.
 * 
 * 
 * TODO:
 *    - Change code to enable removing and replacing
 *      SD card without turning off the ESP32;
 *    - Receive information if the data was updated on cloud or not.
 *      If not, store in a separated file the offline readings; 
 *    - When ESP32 become online again, main code will call a function 
 *      to SDManager_inverter provide to WIFI module the off line readings
 *      to update the cloud server;
 *    - Create function to store QPIRI data (different file name);
 *    - Create function to store other Inverter feedback commands;
 * 
 * Contributors: Larryl79, Richardo Jr ( NiguemJr )
 * Version info: 
 *    - v0.1: Initial implementation for testing only
 **************************************************************************************/
 
#ifndef SDMANAGER_INVERTER_H
#define SDMANAGER_INVERTER_H

#include <string.h>
#include <Arduino.h>
#include "PVinverter.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "time.h"

class SDMANAGER_INVERTER
{
  public:
    SDMANAGER_INVERTER(void) 
    { 
      // Starts SD Card comunication. 
      // This should be moved to another place for enabling 
      // changing the SD card without turn off ESP32
      if(!SD.begin())
      {
        Serial.println("-- ERROR: SDManager: SD Card Mount Failed -----------");
        return ;
      }
    }
  
  void begin(uint8_t _verbose_begin);
  uint8_t sd_StoreQPIGS(PV_INVERTER::pipVals_t _thisPIP, bool _stored_online);
  
  private:
  
    uint8_t _VERBOSE_MODE;
    
    //------- Convert Unixtime to readable date ----------------
    struct tm  ts;

    //------- Private Functions ----------------
   
    uint8_t card_inserted();
  
    // --- SD Sample functions from SD_test.ino example ----
    // --- (https://github.com/espressif/arduino-esp32/tree/master/libraries/SD/examples/SD_Test)
    void testFileIO(fs::FS &fs, const char * path);
    void deleteFile(fs::FS &fs, const char * path);
    void appendFile(fs::FS &fs, const char * path, const char * message);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void readFile(fs::FS &fs, const char * path);

};

#endif
