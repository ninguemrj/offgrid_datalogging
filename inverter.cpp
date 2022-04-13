///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino thread about communicating with the solar inverter: https://forum.arduino.cc/t/rs232-read-data-from-mpp-solar-inverter/600960/72
// Thanks for "athersaleem", "DanX3", "sibianul", "Lerryl79" and my other that shared their knowledge and finds in the Arduino forum.
// Version 1.0 written by Lerryl79 (https://github.com/larryl79/Inverter-Communicator)
// Version 2.0 Copied and changed to fit the Ricardo Jr´s project (https://github.com/ninguemrj/offgrid_datalogging/)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "inverter.h"
#include "SerialDebug.h" //https://github.com/JoaoLopesF/SerialDebug

void INVERTER::begin()
{
  POP_status = "";
}

void INVERTER::store_QPIGS(String value)
{
  
  if (value == "")
  {
    // Update status variables without data when nothing returned from inverter  
      
    pipVals.gridVoltage = 0;  
    pipVals.gridFrequency = 0;
    pipVals.acOutput = 0;
    pipVals.acFrequency = 0;
    pipVals.acApparentPower = 0;
    pipVals.acActivePower = 0;
    pipVals.loadPercent = 0;
    pipVals.busVoltage = 0;
    pipVals.batteryVoltage = 0.0f;
    pipVals.batteryChargeCurrent = 0;
    pipVals.batteryCharge = 0;
    pipVals.inverterTemperature = 0;
    pipVals.PVCurrent = 0.0f;
    pipVals.PVVoltage  = 0;
    pipVals.PVPower    = 0;
    pipVals.batterySCC = 0.0f;
    pipVals.batteryDischargeCurrent = 0;
    String("0000000").toCharArray(pipVals.deviceStatus, 8);
    pipVals.batOffsetFan = 0;
    pipVals.eepromVers = 0;
    pipVals.PV1_chargPower = 0;
    String("000").toCharArray(pipVals.deviceStatus2, 4);
  }
  else
  {
     // Update status with data from inverter    
    char pipInputBuf[500];
    char *val;
    
    strcpy(pipInputBuf, value.c_str());
    
    // Now split the packet into the values
    val = strtok((char *) pipInputBuf, " "); // get the first value
    if (atof(val + 1) >10)   // aviod false value stored, because it shows 2-3V even if grid isn't connected.
      {
        pipVals.gridVoltage = atof(val + 1) * 10;  // Skip the initial '('
      }
      else
      {
        pipVals.gridVoltage = 0;
      }
  
  val = strtok(0, " "); // Get the next value
  pipVals.gridFrequency = atof(val) * 10 ;

  val = strtok(0, " "); // Get the next value
  pipVals.acOutput = atof(val) * 10;

  val = strtok(0, " "); // Get the next value
  pipVals.acFrequency = atof(val) * 10;
  
    val = strtok(0, " "); // Get the next value
    pipVals.acApparentPower = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.acActivePower = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.loadPercent = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.busVoltage = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.batteryVoltage = atof(val)*100;
  
    val = strtok(0, " "); // Get the next value
    pipVals.batteryChargeCurrent = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.batteryCharge = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.inverterTemperature = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.PVCurrent = atof(val)*10;
  
    val = strtok(0, " "); // Get the next value
    pipVals.PVVoltage = atof(val)*10;
  
    pipVals.PVPower= (pipVals.PVVoltage/10) * (pipVals.PVCurrent/10) * 10; // Calculate PV Power
  
    val = strtok(0, " "); // Get the next value
    pipVals.batterySCC = atof(val)*100;
  
    val = strtok(0, " "); // Get the next value
    pipVals.batteryDischargeCurrent = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    strcpy(pipVals.deviceStatus, val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.batOffsetFan = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.eepromVers = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    pipVals.PV1_chargPower = atoi(val);
  
    val = strtok(0, " "); // Get the next value
    strcpy(pipVals.deviceStatus2, String(val).substring(0,3).c_str());
   }

    // Update status with or without data from inverter
    store_status ();
    store_status2 ();
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
  debugV("grid Voltage:......... |%s| V"   , String(pipVals.gridVoltage/10.0).c_str());
  debugV("grid Frequency:....... |%s| Hz"  , String(pipVals.gridFrequency/10.0).c_str());
  debugV("AC Output:............ |%s| V"   , String(pipVals.acOutput/10.0).c_str());
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
	Serial3.print("QRST\r");  //  knock-knock for communiction exist
	Serial3.flush();          // Wait finishing transmitting before going on...
	if (Serial3.readStringUntil('\r') == "(NAKss" )   // check if get response for "knock-knock" from inverter on serial port.
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
  		Serial3.print(inv_command);
  		Serial3.flush();          // Wait finishing transmitting before going on...
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
       str_return = Serial3.readStringUntil('\x0D');

        // TEST for NAK
        // TEST for string lengh
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
      String _resultado = "";
      if (inverter_receive(QPIGS, _resultado) == 0) 
      {
         debugV("INVERTER: QPIGS: command executed successfully. Returned: |%s|", _resultado.c_str());
         
         store_QPIGS(_resultado.c_str());       // split inverter answer and store in pipVals
         inverter_console_data();               // print pipVals on serial port
         inverterData = "";                     // empty string received by inverter
      }
      else
      {
         store_QPIGS("");                       // send empty string to erase previous amounts
         inverter_console_data();               // print pipVals on serial port
         
         // Needs to treat errors for better error messages
         debugE("INVERTER: QPIGS: Error executing the command! Returned: |%s|", _resultado.c_str());    
      }      
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
