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
