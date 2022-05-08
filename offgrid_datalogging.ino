#include "config.h"

/////////////////////////////////
// URGENT
// DATE for SD filename not updated
// SD writing on loop without time delay
/////////////////////////////////////
/*
  offgrid_datalogging
  -------------------
  
  I'm using a Arduino mega+ESP01+RTC DS1307+SD Card shield, to implement data 
  logging and automation project.
  This code uses RS232 Serial commands to acquire energy data from my offgrid
  solar inverter.  
  In the future I will add 16 temperature sensors for monitoring the temperature 
  of each lead acid battery from my battery bank (16 x 100Ah -- 24vx800Ah).
  
  My plans consider: 
     1) Arduino MEGA
        1.1) Concentrates all data acquisition (from inverter and other sensors);
        1.2) Stores all data on SD CARD;
        1.3) Sends all data to a webserver (checking in the SD CARD for non 
             uploaded contents);
        
     2) Raspberry pi 3 or Cloud Server (running a webserver+PHP+mariaDB)
        2.1) Receives the information from Arduino for storing on MariaDB;
        2.2) Provides a webpage with dashboard information from MariaDB data.

        
  ** modified 10 April 2022 by Ricardo Jr (NinguemRJ)
    -> pre-alfa: testing communication with solar inverter, response time, 
       parsing inversor feedback into meaningful variables

Special THANKS for the thread below, on Arduino forum, for all shared knowledge on communicating with solar Inverter, specialy for the user "athersaleem".
https://forum.arduino.cc/t/rs232-read-data-from-mpp-solar-inverter/600960/36

athersaleem´s youtube list with valuable information (Arduino Talking with Solar Inverter):
https://www.youtube.com/watch?v=dlhsc_KjIM4&list=PLDW1zKN_tjaJDp-Ukz8KApchBgA60AFC_


TO DO LIST:
  A.1) function "askInverter": Test the feedback string before returning the QPIGS response string 
       (CRC check and size check -- create a "correct size" constant for each command)
  A.2) Implement other commands between Arduino and inverter (check OUTPUT PRIORITY, and other useful info) 
  B)   create rules to activate OUTPUT PRIORITY as SOLAR or SBU (time rules, AC OUT LOAD rules and etc...)
  C)   start to use SD Card for data logging (define data structure, not uploaded info and etc.)
  D)   start to use wifi (when available) for data logging in the cloud (checking in the SD CARD for non uploaded contents)
  E)   Add 16 temperature sensors with ADS1115 ADC (15/16bits)
  F)   Implement a logic to control how often the averaged data is written in SD/Cloud 
       At this moment it is storing each smoothed read (unix time diff)
  G)   Create (on PVINTERTER.H) accumulated information (watt/h, amp/h)
*/




///// Includes ///////////////////////////////////////////////////////////////////////////////////


#ifndef ARDUINO_H
  #include <Arduino.h>
#endif 

#ifndef PVINVERTER_H
  #include "PVinverter.h"
#endif

#include <Wire.h>
#include "SQLITE_inverter.h"
#include "webserver_inverter.h"


/// Time //////////////////////////////////
#ifndef TIME_H
  #include "time.h"
#endif

#include "sntp.h"

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = -3*3600;
const int   daylightOffset_sec = 0*3600;
time_t _now;
struct tm timeinfo;



////// Variable ///////////////////////////////////////////////////////////////////////////////////


//******* WIFI ************************************************
    String ssid = "Ninguem_house";
    String password = "a2a3a1982a";


//***** SERIAL3 on MEGA for Solar Inverter communication *****************************
//***** SERIAL2 on ESP32 for Solar Inverter communication ****************************
// Change this argument to the SERIAL actualy used to communicates with the inverter
PV_INVERTER inv(Serial2);

//***** Prepare SQLITE variable ***************************************************
SQLITE_INVERTER SQL_inv;

//***** Prepare WEBSERVER variable ***************************************************
WEBSERVER_INVERTER WEB_inv;

//***** var to control when a new smoothed QPIGS reading was taken *******************
uint32_t previous_reading_unixtime = 0;


/// Benchmark
uint32_t oldtime = 0;



String _errorDateTime()
{
  return "-- " + String(timeinfo.tm_year +1900) + "/" + String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_mday) + " - " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec) + " | ";
}


////// Setup ///////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200); // Can change it to 115200, if you want use debugIsr* macros
  delay(500); // Wait a time
  Serial.println(); // To not stay in end of dirty chars in boot
  Serial.println("**** Setup: initializing ...");


//***** PVINVERTER ***************************************************
  // Start inverter class defining serial speed, amount of fields on QPIGS and the #define VERBOSE_MODE
  inv.begin(2400, 2, VERBOSE_MODE);  // "A" = 18 fields from QPIGS / "B" = 22 fields from QPIGS / "C" 22 fields from QPIGS AND QET




//***** Prepare NTC Time client **************************************
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, ntpServer1);
  sntp_init();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

//***** Prepare SQLITE_inverter to store data on SD CARD **********
  SQL_inv.begin();

//***** Prepare WEBSERVER for LIVE data ******************************
// Default web server port = 80
  WEB_inv.begin(ssid, password, &inv, &SQL_inv.SQL_daily_QPIGS);


// TESTING Updates latest 288 QPIGS array from SQLite Averaged Daily
SQL_inv.ask_latest_SQL_QPIGS();


//***** SETUP END
  Serial.println("**** Setup: initialized.");
}

////// Loop /////////////////////////////////////////////////////////////

void loop()
{
  int returned_code = 0;
  // ----------- UPDATE Current DATE/TIME on Loop ---------------------------------
  if (!getLocalTime(&timeinfo)) Serial.println(_errorDateTime() + "-- ERROR: TIME: Failed to obtain time.");

  //ESP32 RTC time 
  //TODO: Syncronize them each hour
  time(&_now);
  
  //--------- Request QPIGS and QPIRI data from inverter  --------------
  returned_code = inv.ask_data(_now, true);
  if (returned_code != 0)                 
  {
    Serial.println(_errorDateTime() + "-- ERROR: MAIN: Error executing 'ask_data' function! Erro code:" + String(returned_code));        
  }

  // --- check for contents on QPIGS_values structure
  if((inv.QPIGS_average.acOutput != 0) && (previous_reading_unixtime != inv.QPIGS_average._unixtime))
  {

    // Prints on console in VERBOSE mode
    if (VERBOSE_MODE == 1) Serial.println(inv.debug_QPIGS(inv.QPIGS_average)); 

    // Store data on SD
    // "_stored_online" fized as "true", as it was not implemented yet
    if (SQL_inv.sd_StoreQPIGS(inv.QPIGS_average, true) != 0)
    {
      Serial.println(_errorDateTime() + "-- ERROR: MAIN: Error executing 'sdStoreQPIGS' function!");        
    }


    // Updated "previous_reading_unixtime" to avoid storing the same data twice
    previous_reading_unixtime = inv.QPIGS_average._unixtime;
  }                  

  ////////////////////////////////////////////////////////////////////////////////////
  // AUTOMATION SETTINGS:
  //
  // Use inverter information and hour/minutes to change 
  // inverter settings considering inverter class rules
  // Uncomment it to use it
  ////////////////////////////////////////////////////////////////////////////////////
  if (inv.handle_automation(timeinfo.tm_hour, timeinfo.tm_min, true) != 0) 
    Serial.println(_errorDateTime() + "-- ERROR: INVERTER: Automation Error! Error code: " + String(returned_code));        

  inv.ESPyield();
  
}

////// FUNCTIONS ///////////////////////////////////////////////////////////////////////////////////


/////////// End
