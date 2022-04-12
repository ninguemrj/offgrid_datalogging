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

#define DEBUG_DISABLED false
//#define DEBUG_INITIAL_LEVEL DEBUG_LEVEL_VERBOSE   

#include "Arduino.h"
#include "Inverter.h"
#include "SerialDebug.h" //https://github.com/JoaoLopesF/SerialDebug
#include <Wire.h>
#include <RTC.h>



////// Variable ///////////////////////////////////////////////////////////////////////////////////

/// RTC
static DS1307 RTC;
static INVERTER inv;

/// Comandos do inversor
String QPIGS = "\x51\x50\x49\x47\x53\xB7\xA9\x0D";
String QMOD = "\x51\x4D\x4F\x44\x96\x1F\x0D";
String QDI = "\x51\x44\x49\x71\x1B\x0D";

/// Storing data from solar inverter
String acIn_volts="";
String acIn_freq="";
String acOut_volts="";
String acOut_freq="";
String acOut_va="";
String acOut_watts="";
String acOut_loadPerc="";
String dcBat_volts="";
String dcBat_chargAmp="";
String dcBat_capPerc="";
String inv_heatSink="";
String pvIn_amp="";
String pvIn_volts="";
String dcBat_voltsSCC="";
String dcBat_dischargAmp="";
String inv_status1="";
String unknown1="";
String unknown2="";
String pvIn_chargPower="";
String inv_status2="";

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
  //debugHandle();

  //--- For benchmarking the Solar inverter communication ---------------
  oldtime=millis();

  //--- Ask the Solar inverter for information ---------------
  //TO DO: function "askInverter": Test the feedback string before returning the response string
  //stemp = askInverter(QPIGS);

  inv.invereter_receive( inv.QPIGS);
  
  // Split the feedback string into meaningfull variables and generate debug (verbose ON) info on serial monitor
  //handleQPIGSfeedback(stemp);


  Serial.print(millis() - oldtime);
  Serial.println(" milisegundos");
  Serial.println(inv.pipVals.gridFrequency);
  
}

////// Setup ///////////////////////////////////////////////////////////////////////////////////

//***** ask solar inverter for data ***************************************************

String askInverter(String cmd){
    String result;

    Serial3.print(cmd);
    result = Serial3.readStringUntil('\r');
//    Serial.println(result);
//    debugV("Reposta inversor: %s", result);  // STRING NOT PRINTED CORRECTLY
  
  return result;
  }



void handleQPIGSfeedback(String stringOne) {

// QPIGS response mapping
//                                1         2         3         4         5         6         7         8         9         10 
//                      01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 
//  String stringOne = "(213.9 60.0 219.8 59.9 0373 0359 010 375 25.50 000 095 0038 00.0 109.2 00.00 00017 01010110 00 00 00000 110$";
//  data field numbers:   1     2     3    4    5     6   7   8    9    10  11  12   13   14    15     16     17    18 19   20  21 CRC

  //1
  acIn_volts = stringOne.substring(1, 6);
  //2
  acIn_freq = stringOne.substring(7, 11 );
  //3
  acOut_volts = stringOne.substring(12, 17);
  //4
  acOut_freq = stringOne.substring(18, 22);
  //5
  acOut_va = stringOne.substring(23, 27);
  //6
  acOut_watts = stringOne.substring(28, 32);
  //7
  acOut_loadPerc = stringOne.substring(33, 36);
  //8 Ignored
  //9
  dcBat_volts = stringOne.substring(41, 46);
  //10 Solar inverter may have 2 or 3 digits in this information depending on MPPT+Utility charger current. Mine have 3 digits (100A).
  dcBat_chargAmp = stringOne.substring(47, 50);
  //11
  dcBat_capPerc = stringOne.substring(51, 54);
  //12
  inv_heatSink = stringOne.substring(55, 59);
  //13
  pvIn_amp = stringOne.substring(60, 64);
  //14
  pvIn_volts = stringOne.substring(65, 70);
  //15
  dcBat_voltsSCC = stringOne.substring(71, 76);
  //16
  dcBat_dischargAmp = stringOne.substring(77, 82);
  //17
  inv_status1 = stringOne.substring(83, 91);
  //18
  unknown1 = stringOne.substring(92, 94);
  //19
  unknown2 = stringOne.substring(95, 97);
  //20
  pvIn_chargPower = stringOne.substring(98, 103);
  //21
  inv_status2 = stringOne.substring(104, 107);

// Only prints if VERBOSE is enabled
    
    debugV("AC IN Volts: |%s|",acIn_volts);
    debugV("AC IN Frequency (Hz): |%s|", acIn_freq);
    debugV("AC OUT Volts: |%s|", acOut_volts);
    debugV("AC OUT Frequency (Hz): |%s|", acOut_freq);
    debugV("AC OUT Apparent Power (va): |%s|",acOut_va);
    debugV("AC OUT Active Power (w): |%s|", acOut_watts);
    debugV("AC OUT Load (%): |%s|", acOut_loadPerc);
    debugV("DC Battery tensao: |%s|", dcBat_volts);
    debugV("DC Battery Charging Current (A): |%s|", dcBat_chargAmp);
    debugV("DC Battery Capacity (%): |%s|", dcBat_capPerc);
    debugV("Heat Sink Temperature (Celcius): |%s|", inv_heatSink);
    debugV("PV IN Current (A): |%s|", pvIn_amp);
    debugV("PV IN Volts : |%s|", pvIn_volts);
    debugV("DC Battery volts (SCC): |%s|", dcBat_voltsSCC);
    debugV("DC Battery Discharging Current (A): |%s|", dcBat_dischargAmp);
    debugV("Inverter Status: |%s|", inv_status1);
    debugV("unknown1: |%s|", unknown1);
    debugV("unknown2: |%s|", unknown2);
    debugV("PV IN Charging Power: |%s|", pvIn_chargPower);
    debugV("Status - Charging to Floating mode: |%s|", inv_status2.substring(0, 1));
    debugV("Status - Switch On: |%s|", inv_status2.substring(1, 2));
    debugV("Status - Dustproof: |%s|", inv_status2.substring(2, 3));

}





/////////// End
