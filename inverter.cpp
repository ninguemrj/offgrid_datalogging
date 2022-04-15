///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino thread about communicating with the solar inverter: https://forum.arduino.cc/t/rs232-read-data-from-mpp-solar-inverter/600960/72
// Thanks for "athersaleem", "DanX3", "sibianul", "Larryl79" and my other that shared their knowledge and finds in the Arduino forum.
// Version 1.0 written by Larryl79 (https://github.com/larryl79/Inverter-Communicator)
// Version 2.0 Copied and changed to fit the Ricardo Jr´s project (https://github.com/ninguemrj/offgrid_datalogging/)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "inverter.h"
#include "SerialDebug.h" //https://github.com/JoaoLopesF/SerialDebug




void INVERTER::begin(uint32_t baudRate)
{
  if (hwStream)
  {
    hwStream->begin(baudRate);
  }
  else
  {
    swStream->begin(baudRate);
  }
  _streamRef = !hwStream? (Stream*)swStream : hwStream;

  // Initialize POP control status flag
  POP_status = "";
  average_count = 0;
  
  //--- For benchmarking the Solar inverter communication ---------------
  average_oldtime=millis();
}

void INVERTER::store_QPIGS(String value)
{
  if (average_count < 10)
  {
      //--- Accumulates readings from 0 to 9 ------------------------------------------
      debugV ("Test countdown : %d", average_count);


      if (value == "")
      {
        //--- QPIGS without data, skip this reading and wait next one -----------------  
         average_count--;  
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
      
        val = strtok(0, " "); // Get the next value
        _pip_average.batOffsetFan = atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.eepromVers = atoi(val);
      
        val = strtok(0, " "); // Get the next value
        _pip_average.PV1_chargPower += atoi(val);
      
        val = strtok(0, " "); // Get the next value
        strcpy(_pip_average.deviceStatus2, String(val).substring(0,3).c_str());
       }
   
      //--- Prepare counting for next array posotion -------------------------------
      average_count++;
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
        pipVals.PV1_chargPower          = _pip_average.PV1_chargPower/10;
        pipVals.batOffsetFan            = _pip_average.batOffsetFan;  // take the lastest read string
        pipVals.eepromVers              = _pip_average.eepromVers;    // take the lastest read string
        strcpy(pipVals.deviceStatus2, _pip_average.deviceStatus2);    // take the lastest read string
        strcpy(pipVals.deviceStatus,  _pip_average.deviceStatus);     // take the lastest read string
        
        //--- Update status with latest read data from inverter ---------------------------
        store_status ();
        store_status2 ();

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
        _pip_average.PV1_chargPower           = 0;



      //--- For benchmarking the averaged Solar inverter communication ---------------------------      
      debugA ("Time to calculate the average %d, count = %d", (millis() - average_oldtime), average_count);
      average_oldtime = millis();

      //--- RESETs average counting -----------------------------------------------------
      average_count = 0;  
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
  debugV("Battery offset Fan:... |%d| V"   , pipVals.batOffsetFan);
  debugV("EEPROM Version:....... |%d|"     , pipVals.eepromVers);
  debugV("PV1 Charger Power:.... |%d| W"   , pipVals.PV1_chargPower);
  debugV("DeviceStatus2:........ |%s|"     , pipVals.deviceStatus2);
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
        debugV("INVERTER: %s : Not recognized command: %s", cmd, str_return);
        return -2;   
      }

      // TEST for CRC receipt match with calculated CRC
      // Different error hangling codes for each one
      
      return 0;
    }
    else
    {
   	  return -1;
	  }
    
}

int INVERTER::ask_inverter_data()
    {
      int funct_return = 0;
      String _resultado = "";
      if (inverter_receive(QPIGS, _resultado) == 0) 
      {
        // checking return string lengh for QPIGS command 
        if (strlen(_resultado.c_str()) < 89)       
        {
          debugE("INVERTER: QPIGS: Receipt string is not completed, size = |%d|.  Returned: %s", strlen(_resultado.c_str()), _resultado.c_str());
          _resultado = "";                                // clear the string result from inverter as it is not complete
          funct_return = -1;                              // short string lengh for QPIGS command 
        }
        else
        {
          debugV("INVERTER: QPIGS: command executed successfully. Returned: |%s|", _resultado.c_str());
          funct_return = 0;                               // Success!!!
        }
      }
      else
      {
         debugE("INVERTER: QPIGS: Error executing the command! Returned: |%s|", _resultado.c_str());    
         _resultado = "";                                 // clear the string result from inverter as an error occured
         funct_return = -2;                               // short string lengh for QPIGS command 
      }      

      store_QPIGS(_resultado.c_str());                    // store in pipVals the inverter response or nothing.
      return (int)funct_return;
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
        
        if (POP_status != POP01)
        {
          // Only changes the Output Priority if previous status is different
          if (inverter_receive(POP01, _resultado) == 0)                   
          {
             debugA("INVERTER: POP01: command executed successfully. Returned: |%s|", _resultado.c_str());
             POP_status = POP01;
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
        
        if (POP_status != POP02)
        {
          // Only changes the Output Priority if previous status is different
          if (inverter_receive(POP02, _resultado) == 0)                   
          {
             debugA("INVERTER: POP02: command executed successfully. Returned: |%s|", _resultado.c_str());
             POP_status = POP02;
          }
          else
          {
            // Needs to treat errors for better error messages
             debugA("INVERTER: POP02: Error executing the command! Returned: |%s|", _resultado.c_str());       
          }
        }
      }
    }
