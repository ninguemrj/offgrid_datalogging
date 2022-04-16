///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino thread about communicating with the solar inverter: https://forum.arduino.cc/t/rs232-read-data-from-mpp-solar-inverter/600960/72
// Thanks for "athersaleem", "DanX3", "sibianul", "Lerryl79" and my other that shared their knowledge and finds in the Arduino forum.
// Version 1.0 written by Lerryl79 (https://github.com/larryl79/Inverter-Communicator)
// Version 2.0 Copied and changed to fit the Ricardo Jr´s project (https://github.com/ninguemrj/offgrid_datalogging/)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "inverter.h"
#include "SerialDebug.h" //https://github.com/JoaoLopesF/SerialDebug




void INVERTER::begin(uint32_t _baudRate, char _protocol) // "A" = 18 fields from QPIGS / "B" = 22 fields from QPIGS 
{
  if (hwStream)
  {
    hwStream->begin(_baudRate);
  }
  else
  {
    swStream->begin(_baudRate);
  }
  _streamRef = !hwStream? (Stream*)swStream : hwStream;

  // Initialize POP control status flag
  _POP_status = "";

  //--- prepare counter for smoothing reads (average)
  _average_count = 0;

  //--- sets how much fields from QPIGS
  _inverter_protocol = _protocol;
  
  //--- For benchmarking the Solar inverter communication ---------------
  _average_oldtime=millis();
}

void INVERTER::store_QPIRI(String value)
{
  if (value == "")
  {
    //--- QPIGS without data, skip this reading and wait next one -----------------  
    pipVals.bat_backToUtilityVolts = 0;
    pipVals.bat_bulkChargeVolts    = 0;
    pipVals.bat_FloatChargeVolts   = 0;
    pipVals.bat_CutOffVolts        = 0;
    pipVals.OutPutPriority         = 0;
    pipVals.ChargerSourcePriority  = 0;  
  }
  else
  {
     //--- Update status with data from inverter  ---------------------------------  
    char pipInputBuf[200];
    char *val;
    
    strcpy(pipInputBuf, value.c_str());
    
    //--- Now split the packet into the values ------------------------------------
    val = strtok((char *) pipInputBuf, " ");            // discart the 1st value
    val = strtok(0, " ");                               // discart the 2nd value
    val = strtok(0, " ");                               // discart the 3th value
    val = strtok(0, " ");                               // discart the 4th value
    val = strtok(0, " ");                               // discart the 5th value
    val = strtok(0, " ");                               // discart the 6th value
    val = strtok(0, " ");                               // discart the 7th value
    val = strtok(0, " ");                               // discart the 8th value
    val = strtok(0, " ");                               // 9th value -> backToUtilityVolts
    pipVals.bat_backToUtilityVolts = atof(val) * 10 ;
    val = strtok(0, " ");                               // 10th value -> bat_CutOffVolts
    pipVals.bat_CutOffVolts = atof(val) * 10 ;
    val = strtok(0, " ");                               // 11th value -> bat_bulkChargeVolts
    pipVals.bat_bulkChargeVolts = atof(val) * 10 ;
    val = strtok(0, " ");                               // 12th value -> bat_FloatChargeVolts
    pipVals.bat_FloatChargeVolts = atof(val) * 10 ;
    val = strtok(0, " ");                               // discart the 13th value
    val = strtok(0, " ");                               // discart the 14th value
    val = strtok(0, " ");                               // discart the 15th value
    val = strtok(0, " ");                               // discart the 16th value
    val = strtok(0, " ");                               // 17th value -> OutPutPriority 
    pipVals.OutPutPriority  = atoi(val);
    val = strtok(0, " ");                               // 18th value -> ChargerSourcePriority
    pipVals.ChargerSourcePriority  = atoi(val);   

   // ignore the other QPIRI fields
    
  }

}

void INVERTER::store_QPIGS(String value)
{
  if (_average_count < 10)
  {
      //--- Accumulates readings from 0 to 9 ------------------------------------------
      debugV ("Test countdown : %d", _average_count);


      if (value == "")
      {
        //--- QPIGS without data, skip this reading and wait next one -----------------  
         _average_count--;  
      }
      else
      {
         //--- Update status with data from inverter  ---------------------------------  
        char pipInputBuf[500];
        char *val;
        
        strcpy(pipInputBuf, value.c_str());
        
        //--- Now split the packet into the values ------------------------------------
        val = strtok((char *) pipInputBuf, " "); // get the first value
        if (atof(val + 1) >10)   // aviod false value stored, because it shows 2-3V even if grid isn't connected.
          {
            _pip_average.gridVoltage += atoi(val + 1);  // Skip the initial '('
          }
          else
          {
            _pip_average.gridVoltage += 0;
          }
      
        val = strtok(0, " "); // Get the next value
        _pip_average.gridFrequency += atof(val) * 10 ;
      
        val = strtok(0, " "); // Get the next value
        _pip_average.acOutput += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.acFrequency += atof(val) * 10;
      
        val = strtok(0, " "); // Get the next value
        _pip_average.acApparentPower += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.acActivePower += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.loadPercent += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.busVoltage += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.batteryVoltage += atof(val)*100;
      
        val = strtok(0, " "); // Get the next value
        _pip_average.batteryChargeCurrent += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.batteryCharge += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.inverterTemperature += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.PVCurrent += atof(val)*10;
      
        val = strtok(0, " "); // Get the next value
        _pip_average.PVVoltage += atof(val)*10;
      
        _pip_average.PVPower += (_pip_average.PVVoltage/10) * (_pip_average.PVCurrent/10) * 10; // Calculate PV Power
      
        val = strtok(0, " "); // Get the next value
        _pip_average.batterySCC += atof(val)*100;
      
        val = strtok(0, " "); // Get the next value
        _pip_average.batteryDischargeCurrent += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        strcpy(_pip_average.deviceStatus, val);
        
        if ( _inverter_protocol == 'B')   // "B" = 22 fields from QPIGS
        {
          val = strtok(0, " "); // Get the next value
          _pip_average.batOffsetFan = atoi(val);
        
          val = strtok(0, " "); // Get the next value
          _pip_average.eepromVers = atoi(val);
        
          val = strtok(0, " "); // Get the next value
          _pip_average.PV1_chargPower += atoi(val);
        
          val = strtok(0, " "); // Get the next value
          strcpy(_pip_average.deviceStatus2, String(val).substring(0,3).c_str());        }
       }
   
      //--- Prepare counting for next array posotion -------------------------------
      _average_count++;
  }
  else
  {
      //--- 10 = calculate average amounts to populate pipvals variables ---------------

        pipVals.gridVoltage             = _pip_average.gridVoltage/10;
        pipVals.gridFrequency           = _pip_average.gridFrequency/10 ;
        pipVals.acOutput                = _pip_average.acOutput/10;
        pipVals.acFrequency             = _pip_average.acFrequency/10;
        pipVals.acApparentPower         = _pip_average.acApparentPower/10;
        pipVals.acActivePower           = _pip_average.acActivePower/10;
        pipVals.loadPercent             = _pip_average.loadPercent/10;
        pipVals.busVoltage              = _pip_average.busVoltage/10;
        pipVals.batteryVoltage          = _pip_average.batteryVoltage/10;
        pipVals.batteryChargeCurrent    = _pip_average.batteryChargeCurrent/10;
        pipVals.batteryCharge           = _pip_average.batteryCharge/10;
        pipVals.inverterTemperature     = _pip_average.inverterTemperature/10;
        pipVals.PVCurrent               = _pip_average.PVCurrent/10;
        pipVals.PVVoltage               = _pip_average.PVVoltage/10;
        pipVals.PVPower                 = _pip_average.PVPower/10;
        pipVals.batterySCC              = _pip_average.batterySCC /10;
        pipVals.batteryDischargeCurrent = _pip_average.batteryDischargeCurrent/10;
        strcpy(pipVals.deviceStatus,  _pip_average.deviceStatus);     // take the lastest read string

        if ( _inverter_protocol == 'B')   // "B" = 22 fields from QPIGS
        {
          pipVals.PV1_chargPower          = _pip_average.PV1_chargPower/10;
          pipVals.batOffsetFan            = _pip_average.batOffsetFan;  // take the lastest read string
          pipVals.eepromVers              = _pip_average.eepromVers;    // take the lastest read string
          strcpy(pipVals.deviceStatus2, _pip_average.deviceStatus2);    // take the lastest read string
          
          //--- Update status2 with latest read data from inverter ---------------------------
          store_status2 ();
        }
        
        //--- Update status with latest read data from inverter ---------------------------
        store_status ();

        //--- RESETs the _pip_average values to not accummulate the next readings with previous ones ----
        _pip_average.gridVoltage              = 0;
        _pip_average.gridFrequency            = 0;
        _pip_average.acOutput                 = 0;
        _pip_average.acFrequency              = 0;
        _pip_average.acApparentPower          = 0;
        _pip_average.acActivePower            = 0;
        _pip_average.loadPercent              = 0;
        _pip_average.busVoltage               = 0;
        _pip_average.batteryVoltage           = 0;
        _pip_average.batteryChargeCurrent     = 0;
        _pip_average.batteryCharge            = 0;
        _pip_average.inverterTemperature      = 0;
        _pip_average.PVCurrent                = 0;
        _pip_average.PVVoltage                = 0;
        _pip_average.PVPower                  = 0;
        _pip_average.batterySCC               = 0;
        _pip_average.batteryDischargeCurrent  = 0;

         if ( _inverter_protocol == 'B')   // "B" = 22 fields from QPIGS
        {
          _pip_average.PV1_chargPower           = 0;
        }




      //--- RESETs average counting -----------------------------------------------------
      _average_count = 0;  
  }
  

}

void INVERTER::store_status ()
{
  char val[8];
  strcpy(val, pipVals.deviceStatus);		// get the first value
  DevStatus.SBUpriority      = val[0];
  DevStatus.ConfigStatus     = val[1];		// configuration status: 1: Change 0: unchanged b6
  DevStatus.FwUpdate         = val[2];      // b5: SCC firmware version 1: Updated 0: unchanged
  DevStatus.LoadStatus       = val[3];      // b4: Load status: 0: Load off 1:Load on
  DevStatus.BattVoltSteady   = val[4];		// b3: battery voltage to steady while charging
  DevStatus.Chargingstatus   = val[5];		// b2: Charging status( Charging on/off)
  DevStatus.SCCcharge        = val[6];		// b1: Charging status( SCC charging on/off)
  DevStatus.ACcharge         = val[7];      // b0: Charging status(AC charging on/off)
}

void INVERTER::store_status2 ()
{
  char val[4];
  strcpy(val, pipVals.deviceStatus2);		// get the first value

  DevStatus2.changingFloatMode			 = val[0] ;		// 10: flag for charging to floating mode
  DevStatus2.SwitchOn       				 = val[1] ;		// b9: Switch On
  DevStatus2.dustProof			  	     = val[2] ;		// b8: flag for dustproof installed(1-dustproof installed,0-no dustproof, only available for Axpert V series)
}

void INVERTER::inverter_console_data()
{
  debugV("grid Voltage:......... |%d| V"   , pipVals.gridVoltage);
  debugV("grid Frequency:....... |%s| Hz"  , String(pipVals.gridFrequency/10.0).c_str());
  debugV("AC Output:............ |%d| V"   , pipVals.acOutput);
  debugV("AC Frequency:......... |%s| Hz"  , String(pipVals.acFrequency/10.0).c_str());
  debugV("AC ApparentPower:..... |%d| VA"  , pipVals.acApparentPower);
  debugV("AC ActivePower:....... |%d| W"   , pipVals.acActivePower);
  debugV("load Percent:......... |%d| %"   , pipVals.loadPercent);
  debugV("bus Voltage:.......... |%d| V"   , pipVals.busVoltage); 
  debugV("battery Voltage:...... |%s| V"   , String(pipVals.batteryVoltage/100.00).c_str());
  debugV("battery ChargeCurrent: |%d| A"   , pipVals.batteryChargeCurrent); 
  debugV("battery Charge:....... |%d| %"   , pipVals.batteryCharge); 
  debugV("inverter Temperature:. |%d| C"   , pipVals.inverterTemperature); 
  debugV("PV Current:........... |%s| A"   , String(pipVals.PVCurrent /10.0).c_str());
  debugV("PV Voltage:........... |%s| V"   , String(pipVals.PVVoltage /10.0).c_str()); 
  debugV("PV Power:............. |%s| W"   , String(pipVals.PVPower   /10.0).c_str());  
  debugV("Battery SCC:.......... |%s| V"   , String(pipVals.batterySCC/100.00).c_str()); 
  debugV("Batt DischargeCurrent: |%d| A"   , pipVals.batteryDischargeCurrent); 
  debugV("DeviceStatus:......... |%s|"     , pipVals.deviceStatus);
  
  if ( _inverter_protocol = 'B')   // "B" = 22 fields from QPIGS
  {
    debugV("Battery offset Fan:... |%d| V"   , pipVals.batOffsetFan);
    debugV("EEPROM Version:....... |%d|"     , pipVals.eepromVers);
    debugV("PV1 Charger Power:.... |%d| W"   , pipVals.PV1_chargPower);
    debugV("DeviceStatus2:........ |%s|"     , pipVals.deviceStatus2);
  }

  debugV("Bat Back to Grid:..... |%s| V"   , String(pipVals.bat_backToUtilityVolts/10.0).c_str()); 
  debugV("Bat Bulk Charge:...... |%s| V"   , String(pipVals.bat_bulkChargeVolts/10.0).c_str()); 
  debugV("Bat Float Charge:..... |%s| V"   , String(pipVals.bat_FloatChargeVolts/10.0).c_str()); 
  debugV("Bat CutOff:........... |%s| V"   , String(pipVals.bat_CutOffVolts/10.0).c_str()); 
  debugV("Output Priority:...... |%d| 0: Utility first / 1: Solar first / 2: SBU first"   , pipVals.OutPutPriority); 
  debugV("Charging Priority:.... |%d| 0: Utility first / 1: Solar first / 2: Solar + Utility / 3: Only solar"   , pipVals.ChargerSourcePriority); 
}

// ******************************************  CRC Functions  ******************************************
uint16_t INVERTER::crc_xmodem_update (uint16_t crc, uint8_t data)
{
  int i;
  crc = crc ^ ((uint16_t)data << 8);
  for (i=0; i<8; i++) {
  if (crc & 0x8000)
    crc = (crc << 1) ^ 0x1021; //(polynomial = 0x1021)
  else
    crc <<= 1;
   }
return crc;
}

uint16_t INVERTER::calc_crc(char *msg, int n)
{
// See bottom of this page: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
// Polynomial: x^16 + x^12 + x^5 + 1 (0x1021)
// Initial value. xmodem uses 0xFFFF but this example
// requires an initial value of zero.
  uint16_t x = 0;
    while( n-- ) {
    x = crc_xmodem_update(x, (uint16_t)*msg++);
    yield();
    }
      return(x);
}

// ******************************************  inverter communication  *********************************

int INVERTER::inverter_send(String inv_command)
{
	_streamRef->print("QRST\r");  //  knock-knock for communiction exist
	_streamRef->flush();          // Wait finishing transmitting before going on...
	if (_streamRef->readStringUntil('\r') == "(NAKss" )   // check if get response for "knock-knock" from inverter on serial port.
	{
 /* 		uint16_t vgCrcCheck;
  		int vRequestLen = 0;
  		char s[6];
  		int xx; 
  		int yy;
  
  		vRequestLen = inv_command.length();
  		char vRequestArray[vRequestLen]; //Arrary define
  		inv_command.toCharArray(vRequestArray, vRequestLen + 1);
  
  		//Calculating CRC
  		vgCrcCheck = calc_crc(vRequestArray,vRequestLen);
  
  		// CRC returns two characters - these need to be separated and send as HEX to Inverter
  		String vgCrcCheckString = String(vgCrcCheck, HEX);
  		String vCrcCorrect = vgCrcCheckString.substring(0,2) + " " + vgCrcCheckString.substring(2,4);
  			
  		//CRC are returned as B7A9 - need to separate them and change from ASCII to HEX
  		vCrcCorrect.toCharArray(s, 6);
  		sscanf(s, "%x %x", &xx, &yy);  
  
  		inv_command += char(xx);   // add CRC byte 1
  		inv_command += char(yy);   // add CRC byte 2
 
*/
      inv_command += "\x0D";     // add CR
  		//Sending Request to inverter
  		_streamRef->print(inv_command);
  		_streamRef->flush();          // Wait finishing transmitting before going on...
  }
  else
  {
		return -1; // No serial communication
  }
   return 0; // NAKss returned, serial communication up and running
}

int INVERTER::inverter_receive( String cmd, String& str_return )
{
  if ( inverter_send(cmd)==0 )
    {
      str_return = _streamRef->readStringUntil('\x0D');
      
      // checking Command not recognized 
      if (str_return == "(NAKss") 
      {
        debugE("INVERTER: %s : Not recognized command: %s", cmd.c_str(), str_return.c_str());
        return -2;   
      }

      // TODO: TEST for CRC receipt match with calculated CRC
      
      debugV("INVERTER: %s : Command executed successfully. Returned: %s", cmd.c_str(), str_return.c_str());
      return 0;
    }
    else
    {
      // No serial communication
      debugE("INVERTER: %s : No serial communication", cmd.c_str());
      str_return = "";
   	  return -1;
	  }
    
}

void INVERTER::ask_inverter_QPIRI( String& _result)
  {
      int _funct_return = 0;
      _result = "";
      _funct_return = inverter_receive(QPIRI, _result);
      if (_funct_return == 0) 
      {
        // checking return string lengh for QPIRI command 
        if (strlen(_result.c_str()) < 85)       
        {
          debugE("INVERTER: QPIRI: Receipt string is not completed, size = |%d|.  Returned: %s", strlen(_result.c_str()), _result.c_str());
          _result = "";                                    // clear the string result from inverter as it is not complete
          _funct_return = -1;                              // short string lengh for QPIRI command 
        }
      }
//      store_QPIGS(_result.c_str());                    // store in pipVals the inverter response or nothing.
//      return (int)_funct_return;    
  }

int INVERTER::ask_inverter_data()
    {
      int _funct_return = 0;
      String _result = "";
      _funct_return = inverter_receive(QPIGS, _result);
      if (_funct_return == 0) 
      {
        debugV("INVERTER: QPIGS: Receipt string size = |%d|.  Returned: %s", strlen(_result.c_str()), _result.c_str());
        // checking return string lengh for QPIGS command 
        if (strlen(_result.c_str()) < 85)       
        {
          debugE("INVERTER: QPIGS: Receipt string is not completed, size = |%d|.  Returned: %s", strlen(_result.c_str()), _result.c_str());
          _result = "";                                  // clear the string result from inverter as it is not complete
          _funct_return = -1;                            // short string lengh for QPIGS command 
        }
      }
      store_QPIGS(_result.c_str());                      // average and store in pipVals the inverter response or nothing.

      // Ask Inverer for QPIRI configuration in the 10th QPIGS reading (0 to 9)
      // (when the averaged amount will be stored in the public variables)
      if (_average_count == 9)
      {
        String _QPIRI_result;
        // Ask Inverer for QPIRI
        ask_inverter_QPIRI(_QPIRI_result);

        // store QPIRI info
        store_QPIRI(_QPIRI_result);

        //--- For benchmarking the averaged Solar inverter communication ---------------------------      
        debugA ("Time to calculate the average %d, count = %d", (millis() - _average_oldtime), _average_count);
        _average_oldtime = millis();
        
      }
      
      return (int)_funct_return;    
    }

int INVERTER::handle_inverter_automation(int _hour, int _min)
    {
      String _resultado = "";
      uint32_t minutes = (_hour * 60) + _min ;              // Minutes to compare with preset time rules
      
      const uint32_t _evening_min = ( 18   * 60) +   0  ;   // 18:00hs evening start (sun stops to generating on 16:30, but 
                                                            // I just use battery as from 18hs to save them from deep discharge
                                                       
      const uint32_t _begin_min     = (  7   * 60) +   30 ; // 07:30hs start generating solar power

      // RULE #1 //////////////////////////////////////////////////////////
      //  - After "_begin_min" and before "_evening_min"
      //
      //  SET: SOLAR = Solar First (Solar, Utility and Battery) 
      /////////////////////////////////////////////////////////////////////
       
      if ((_begin_min < minutes) && (minutes < _evening_min))
      {
        // Informed time is between sun rise and evening = set as "Solar First"
        
        if (_POP_status != POP01)
        {
          // Only changes the Output Priority if previous status is different
          if (inverter_receive(POP01, _resultado) == 0)                   
          {
             debugA("INVERTER: POP01: command executed successfully. Returned: |%s|", _resultado.c_str());
             _POP_status = POP01;
          }
          else
          {
             // Needs to treat errors for better error messages
             debugA("INVERTER: POP01: Error executing the command! Returned: |%s|", _resultado.c_str());       
          }
        }
      }

      // RULE #2 //////////////////////////////////////////////////////////
      //  - Before "_begin_min" OR after "_evening_min"
      //
      //  SET: SBU (Solar, Battery and Utility) 
      /////////////////////////////////////////////////////////////////////
       
      if ((_begin_min > minutes) || (minutes > _evening_min))
      {
        // Informed time is between sun rise and evening = set as "Solar First"
        
        if (_POP_status != POP02)
        {
          // Only changes the Output Priority if previous status is different
          if (inverter_receive(POP02, _resultado) == 0)                   
          {
             debugA("INVERTER: POP02: command executed successfully. Returned: |%s|", _resultado.c_str());
             _POP_status = POP02;
          }
          else
          {
            // Needs to treat errors for better error messages
             debugA("INVERTER: POP02: Error executing the command! Returned: |%s|", _resultado.c_str());       
          }
        }
      }
    }
