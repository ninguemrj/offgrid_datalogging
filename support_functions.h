/**************************************************************************************
 * support_funtions.cpp (v0.1)
 * Library to be used (or not) with the PV_inverter library.
 * Created to store read data in SD card for backup purposes.
 * 
 * 
 * TODO:
 *    - Define a standard begin for each type of message (verbose prints all)
 *    - Create property and functions to change verbose mode on runtime
 *    - add date and time before messages
 *     
 * Version info: 
 *    - v0.1: Initial implementation for testing only
 **************************************************************************************/
 
#ifndef SUPPORT_FUNCTIONS_H
#define SUPPORT_FUNCTIONS_H

#ifndef ARDUINO_H
  #include <Arduino.h>
#endif 

#ifndef STRING_H
  #include <string.h>
#endif


#ifndef TIME_H
  #include "time.h"
  #include "TimeLib.h"
#endif

#include "config.h"

class SUPPORT_FUNCTIONS
{
  public:
    static void logMsg(int _level, String _msg);  // 0 = INFO / 1 = Warning / 2 = ERROR / 3 = Debug / 4 = Benchmark
    static time_t convertToUnixtime(uint16_t _year, uint8_t _month, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec);

  private:
    //time_t _unixtime; // a timestamp

};

#endif
