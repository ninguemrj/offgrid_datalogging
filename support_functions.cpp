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
 * Version info: 
 *    - v0.1: Initial implementation for testing only
 **************************************************************************************/
 
#include "SQLite_inverter.h"



// 0 = INFO / 1 = Warning / 3 = ERROR / 4 = Debug-Verbose
void SUPPORT_FUNCTIONS::debugMsg(int _level, char _msg)
{
  if (_VERBOSE_MODE == 1) 
  {
    Serial.println(_msg);
  }
}


// 0 = INFO / 1 = Warning / 3 = ERROR / 4 = Debug-Verbose
void SUPPORT_FUNCTIONS::debugMsg(int _level, String _msg)
{
  if (_VERBOSE_MODE == 1) 
  {
    Serial.println(_msg);
  }
}
