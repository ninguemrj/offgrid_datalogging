//#define DEBUG_DISABLED true
//#define DEBUG_INITIAL_LEVEL DEBUG_LEVEL_VERBOSE   

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

/// Personel INVERTER class
static INVERTER inv;
/// Benchmark
uint32_t oldtime = 0;



////// Setup ///////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(500000); // Can change it to 115200, if you want use debugIsr* macros
  delay(500); // Wait a time
  Serial.println(); // To not stay in end of dirty chars in boot

  debugA("**** Setup: initializing ...");

//***** RTC ***************************************************
  RTC.begin();

  if (!RTC.isRunning())
  {
    debugE("Real Time Clock not working!");
  }
// Pending ...
// Present start time as debugA



//***** SERIAL3 on MEGA for Solar Inverter communication ***************************************************
  
  Serial3.begin(2400);
  Serial3.setTimeout(800);
// Pending ...
// Check and present Serial3 status


//***** WIFI Connection ***************************************************
// Pending ...
// Present IP Addr
// SSID
// Connection status
    
    
//***** SD Card  ***************************************************
// Pending ...
// Check if SD card is present (on loop too, debugE if not)
// Check if SD card is writable (on loop too, debugE if not)


//***** SETUP END
  debugA("**** Setup: initialized.");
}



////// Loop /////////////////////////////////////////////////////////////

void loop()
{
  // stemp for testing pourposes only, it will be remeved later
  //String stemp = "";

  //--- SERIAL DEBUGGER Handle ---------------
  debugHandle();

  //--- For benchmarking the Solar inverter communication ---------------
  oldtime=millis();

  //--- Ask the Solar inverter for information ---------------
  //TO DO: function "askInverter": Test the feedback string before returning the response string
  //stemp = askInverter(QPIGS);

  if (inv.invereter_receive( inv.QPIGS)!=0)
  {
     Serial.println("Handling communication error no prepared...");
  };
  
  // Split the feedback string into meaningfull variables and generate debug (verbose ON) info on serial monitor
  //handleQPIGSfeedback(stemp);


  Serial.print("Time spent on LOOP: ");
  Serial.print(millis() - oldtime);
  Serial.println("ms");
  //Serial.println(inv.pipVals.gridFrequency);
  
}

////// FUNCTIONS ///////////////////////////////////////////////////////////////////////////////////


/////////// End
