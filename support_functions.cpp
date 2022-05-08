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
 
#include "support_functions.h"



// 0 = INFO / 1 = Warning / 2 = ERROR / 3 = Debug / 4 = Benchmark
void SUPPORT_FUNCTIONS::logMsg(int _level, String _msg)
{
  String _formatted_msg = "";
  switch ( _level )     // select protocol for the right CRC calculation.
  {
    case 0:  
    {
      _formatted_msg = "--I-- INFO: " + _msg;   
    }
    break;

    case 1:
    {
      _formatted_msg = "--W-- WARNING: " + _msg;   
    }
    break;
    
    case 2:   
    {
      _formatted_msg = "--E-- ERROR: " + _msg;   
    }
    break;
    
    case 3:   
    {
      _formatted_msg = "--D-- DEBUG: " + _msg;   
    }
    break;
    
    case 4:   // for future
    {
      _formatted_msg = "--B-- Benchmark: " + _msg;   
    }
    break;
  }    
    
  switch ( VERBOSE_OUTPUT )
  {
    case 0:  
    {
      if ((VERBOSE_MODE == 0) && ((_level == 0) || (_level == 1) || (_level == 2)))                                     Serial.println(_formatted_msg);
      if ((VERBOSE_MODE == 1) && ((_level == 0) || (_level == 1) || (_level == 2) || (_level == 3)))                    Serial.println(_formatted_msg);
      if ((VERBOSE_MODE == 2) && ((_level == 0) || (_level == 1) || (_level == 2) || (_level == 4)))                    Serial.println(_formatted_msg);
      if ((VERBOSE_MODE == 3) && ((_level == 0) || (_level == 1) || (_level == 2) || (_level == 3) || (_level == 4)))   Serial.println(_formatted_msg);
    }
    break;
  
    case 1:
    {
      // SD CARD TO BE IMPLEMENTED   
    }
    break;
  }
}


/*--- convertToUnixtime----------------------------------------------------------------------------
* Receives date/time in pieces abd converts to Unix timestamp (1970)*
*
*-------------------------------------------------------------------------------------------------*/
uint32_t SUPPORT_FUNCTIONS::convertToUnixtime(uint8_t _year, uint8_t _month, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec) 
{
  // convert a date and time into unix time, offset 1970
  _time_elements.Second = _sec;
  _time_elements.Minute = _min;
  _time_elements.Hour = _hour;
  _time_elements.Day = _day;
  _time_elements.Month = _month - 1;      // months start from 0, so deduct 1
  _time_elements.Year = _year - 1970; // years since 1970, so deduct 1970
  return (uint32_t)makeTime(_time_elements);
}
