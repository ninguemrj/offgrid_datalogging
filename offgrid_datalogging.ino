#define VERBOSE_MODE 0     // 0 = none  / 1 = Debug
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

*/




///// Includes ///////////////////////////////////////////////////////////////////////////////////


#include "Arduino.h"
#include "SerialDebug.h" //https://github.com/JoaoLopesF/SerialDebug
#include "Inverter.h"
#include <Wire.h>
#include <RTC.h>



////// Variable ///////////////////////////////////////////////////////////////////////////////////

/// RTC
static DS1307 RTC;

//***** SERIAL3 on MEGA for Solar Inverter communication ***************************************************
// Change this argument to the SERIAL actualy used to communicates with the inverter
static INVERTER inv(Serial3);


/// Benchmark
uint32_t oldtime = 0;



////// Setup ///////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(500000); // Can change it to 115200, if you want use debugIsr* macros
  delay(500); // Wait a time
  Serial.println(); // To not stay in end of dirty chars in boot

  Serial.println("**** Setup: initializing ...");

//***** RTC ***************************************************
  RTC.begin();

  if (!RTC.isRunning())
  {
    Serial.println("-- ERROR: Real Time Clock not working!");
  }
// Pending ...
// Present start time as debugA



// Start inverter class defining serial speed, amount of fields on QPIGS and the #define VERBOSE_MODE
  inv.begin(2400, 'B', VERBOSE_MODE);  // "A" = 18 fields from QPIGS / "B" = 22 fields from QPIGS 



//***** WIFI Connection ***************************************************
// Pending ...
// Present IP Addr
// SSID
// Connection status
    
    
//***** SD Card  ***************************************************
// Pending ...
// Check if SD card is present 
// Check if SD card is writable


//***** SETUP END
  Serial.println("**** Setup: initialized.");
}



////// Loop /////////////////////////////////////////////////////////////

void loop()
{
  int returned_code = 0;
  
  //// request QPIGS and QPIRI data from inverter  /////////////////////////////////////////////
  returned_code = inv.ask_inverter_data();
  if (returned_code != 0)                 
  {
    Serial.println("-- ERROR: INVERTER: Error executing 'ask_inverter_data' function! Erro code:" + String(returned_code));        
  }

  // print pipVals on serial port only on VERBOSE mode
  if(inv.pipVals.acOutput != 0)
    inv.inverter_console_data();                     

  ////////////////////////////////////////////////////////////////////////////////////
  // AUTOMATION SETTINGS:
  //
  // Use inverter information and hour/minutes to change 
  // inverter settings considering inverter class rules
  // Uncomment it to use it
  ////////////////////////////////////////////////////////////////////////////////////
//  if (inv.handle_inverter_automation(19, 23) != 0) 
//    Serial.println("-- ERROR: INVERTER: Automation Error! Error code: " + String(returned_code));        
    
  
}

////// FUNCTIONS ///////////////////////////////////////////////////////////////////////////////////


/////////// End
