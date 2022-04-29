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
 
#include "SDManager_inverter.h"

////////// Date and time function for error mesages //////////
extern String _errorDateTime();


void SDMANAGER_INVERTER::begin(uint8_t _verbose_begin)
{
    // Sharing the same verbose mode from main code
    _VERBOSE_MODE = _verbose_begin;
}

uint8_t SDMANAGER_INVERTER::sd_StoreQPIGS(PV_INVERTER::pipVals_t _thisPIP, bool _stored_online)
{
  //------------   Check if there is SD and available size ----------------------
  if (card_inserted() != 0) 
  {
    Serial.println(_errorDateTime() + "-- ERROR: SDManager: QPIGS not stored in SD Card! -----");
    return 1;
  }

  // Convert unixtime from smoothed read to readable date to define file name
  ts = *localtime((time_t*)_thisPIP._unixtime);
  
  //------- file name ----------------

  String _file = "/"+String(ts.tm_year+1900)+String(ts.tm_mon+1)+String(ts.tm_mday)+"_QPIGS.csv";

  //----- Prepared QPIGS string for writing ----------------
  String _QPIGS_line = 
      String(_thisPIP._unixtime)                + "," +
      String(_thisPIP.gridVoltage)              + "," +
      String(_thisPIP.gridFrequency)            + "," +
      String(_thisPIP.acOutput)                 + "," +
      String(_thisPIP.acFrequency)              + "," +
      String(_thisPIP.acApparentPower)          + "," +
      String(_thisPIP.acActivePower)            + "," +
      String(_thisPIP.loadPercent)              + "," +
      String(_thisPIP.busVoltage)               + "," +
      String(_thisPIP.batteryVoltage)           + "," +
      String(_thisPIP.batteryChargeCurrent)     + "," +
      String(_thisPIP.batteryCharge)            + "," +
      String(_thisPIP.inverterTemperature)      + "," +
      String(_thisPIP.PVCurrent)                + "," +
      String(_thisPIP.PVVoltage)                + "," +
      String(_thisPIP.PVPower)                  + "," +
      String(_thisPIP.batterySCC)               + "," +
      String(_thisPIP.batteryDischargeCurrent)  + "," +
      _thisPIP.deviceStatus[0]                  + "," +
      _thisPIP.deviceStatus[1]                  + "," +
      _thisPIP.deviceStatus[2]                  + "," +
      _thisPIP.deviceStatus[3]                  + "," +
      _thisPIP.deviceStatus[4]                  + "," +
      _thisPIP.deviceStatus[5]                  + "," +
      _thisPIP.deviceStatus[6]                  + "," +
      _thisPIP.deviceStatus[7]                  + "," +
      String(_thisPIP.batOffsetFan)             + "," +
      String(_thisPIP.eepromVers)               + "," +
      String(_thisPIP.PV1_chargPower)           + "," +
      _thisPIP.deviceStatus2[0]                 + "," +
      _thisPIP.deviceStatus2[1]                 + "," +
      _thisPIP.deviceStatus2[2];

    if (_VERBOSE_MODE == 1)
    {
      Serial.println(_errorDateTime() + "-- VERBOSE: SDManager: File name: " + _file);
      Serial.println(_errorDateTime() + "-- VERBOSE: SDManager: QPIGS String |" + _QPIGS_line + "|");
    }

  appendFile(SD, _file.c_str(), _QPIGS_line.c_str());
  return 0;

}


/// PRIVATE FUNCTIONS ///////////////////////////////////////////////////

uint8_t SDMANAGER_INVERTER::card_inserted()
{
    if(SD.cardType() == CARD_NONE){
        Serial.println(_errorDateTime() + "-- ERROR: SDManager: No SD card attached.");
        return 1;
    }
    
    // --- only 0.5mb available ERROR
    if((SD.cardSize() - SD.usedBytes()) < (500 * 1024) )
    {
        Serial.println(_errorDateTime() + "--- ERROR: SDManager: less than 500kb available, STORE function not executed!");
        return 2;
    }

    // --- less then 100mb available warning
    if((SD.cardSize() - SD.usedBytes()) < (100 * 1024 * 1024) )
    {
        Serial.println(_errorDateTime() + "--- WARNING: SDManager: less than 100mb available, please change SD Card!");
    }    
    
    if (_VERBOSE_MODE == 1)
    {
      Serial.print(_errorDateTime() + "--- VERBOSE: SDManager_inverter: SD Card Size: ");
      Serial.print(SD.cardSize()/1024/1024.00);
      Serial.print(" Mb | Available: ");
      Serial.print((SD.cardSize() - SD.usedBytes()) /1024/1024.00);
      Serial.print(" Mb | Used: ");
      Serial.print(SD.usedBytes() /1024/1024.00);
      Serial.println(" Mb ");
    }
    return 0;
}


// Missing better implementation: Read line by line into an feedback array

void SDMANAGER_INVERTER::readFile(fs::FS &fs, const char * path){
   File file = fs.open(path);
    if(!file){
        Serial.print("--- ERROR: SDManager_Inverter: Failed to open the file for reading: ");
        Serial.println(path);
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void SDMANAGER_INVERTER::writeFile(fs::FS &fs, const char * path, const char * message)
{
    File file = fs.open(path, FILE_WRITE);
    if(!file)
    {
        Serial.print("--- ERROR: SDManager_Inverter: Failed to open file for writing: ");
        Serial.println(path);
        return;
    }
    if(!file.print(message))
    {
        Serial.print("--- ERROR: SDManager_Inverter: Failed to write data in the file: ");
        Serial.println(path);
    }
    file.close();
}

void SDMANAGER_INVERTER::appendFile(fs::FS &fs, const char * path, const char * message)
{
    File file = fs.open(path, FILE_APPEND);
    if(!file)
    {
        Serial.print("--- ERROR: SDManager_Inverter: Failed to open file for appending: ");
        Serial.println(path);
        return;
    }
    if(!file.println(message))
    {
        Serial.print("--- ERROR: SDManager_Inverter: Failed to append data in the file: ");
        Serial.println(path);
    }
    file.close();
}

void SDMANAGER_INVERTER::deleteFile(fs::FS &fs, const char * path)
{
    if(!fs.remove(path))
    {
        Serial.print("--- ERROR: SDManager_Inverter: Failed to delete file: ");
        Serial.println(path);
    }
}
