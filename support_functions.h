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
#endif


class SUPPORT_FUNCTIONS
{
  public:
    void debugMsg(int _level, char _msg, int _VERBOSE_MODE);    // 0 = INFO / 1 = Warning / 3 = ERROR / 4 = Debug-Verbose
    void debugMsg(int _level, String _msg, int _VERBOSE_MODE);  // 0 = INFO / 1 = Warning / 3 = ERROR / 4 = Debug-Verbose


};

#endif
