#include "inverter.h"
#include "SerialDebug.h" //https://github.com/JoaoLopesF/SerialDebug


void INVERTER::store_val(String cmd)
{
  char pipInputBuf[500];
  char *val;
  
  strcpy(pipInputBuf, inverterData.c_str());
  // Now split the packet into the values
   if ( cmd == QPIGS )
  {
  val = strtok((char *) pipInputBuf, " "); // get the first value
  if (atoi(val + 1) >10)   // aviod false value stored, because it shows 2-3V even if grid isn't connected.
    {
    pipVals.gridVoltage = atoi(val + 1);  // Skip the initial '('
    }
    else
    {
    pipVals.gridVoltage = 0;
    }

  pipVals.gridFrequency = strtok(0, " "); // Get the next value

  val = strtok(0, " "); // Get the next value
  pipVals.acOutput = atoi(val);

  pipVals.acFrequency = strtok(0, " "); // Get the next value

  val = strtok(0, " "); // Get the next value
  pipVals.acApparentPower = atoi(val);

  val = strtok(0, " "); // Get the next value
  pipVals.acActivePower = atoi(val);

  val = strtok(0, " "); // Get the next value
  pipVals.loadPercent = atoi(val);

  val = strtok(0, " "); // Get the next value
  pipVals.busVoltage = atoi(val);

  val = strtok(0, " "); // Get the next value
  pipVals.batteryVoltage = atof(val);

  val = strtok(0, " "); // Get the next value
  pipVals.batteryChargeCurrent = atoi(val);

  val = strtok(0, " "); // Get the next value
  pipVals.batteryCharge = atoi(val);

  val = strtok(0, " "); // Get the next value
  pipVals.inverterTemperature = atoi(val);

  val = strtok(0, " "); // Get the next value
  pipVals.PVCurrent = atof(val);

  val = strtok(0, " "); // Get the next value
  pipVals.PVVoltage = atoi(val);

  pipVals.PVPower= pipVals.PVVoltage * pipVals.PVCurrent; // Calculate PV Power

  val = strtok(0, " "); // Get the next value
  pipVals.batterySCC = atof(val);

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
  strcpy(pipVals.deviceStatus2, val);
  }

   if ( cmd == QPI )
    {
        
    }
}

void INVERTER::store_status ()
{
  char val[8];
  strcpy(pipVals.deviceStatus, val);		// get the first value
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
  char val[3];
  strcpy(pipVals.deviceStatus, val);		// get the first value

  DevStatus2.changingFloatMode			 = val[0] ;		// 10: flag for charging to floating mode
  DevStatus2.SwitchOn       				 = val[1] ;		// b9: Switch On
  DevStatus2.dustProof			  	     = val[2] ;		// b8: flag for dustproof installed(1-dustproof installed,0-no dustproof, only available for Axpert V series)
}

void INVERTER::inverter_console_data(String cmd)
{
  if (cmd==QPIGS)
  {
  // Print out readings

  
  debugV("grid Voltage:......... |%d| V"   , pipVals.gridVoltage);
  debugV("grid Frequency:....... |%s| Hz"  , pipVals.gridFrequency.c_str());
  debugV("AC Output:............ |%d| V"   , pipVals.acOutput);
  debugV("AC Frequency:......... |%s| Hz"  , pipVals.acFrequency.c_str());
  debugV("AC ApparentPower:..... |%d| VA"  , pipVals.acApparentPower);
  debugV("AC ActivePower:....... |%d| W"   , pipVals.acActivePower);
  debugV("load Percent:......... |%d| %"   , pipVals.loadPercent);
  debugV("bus Voltage:.......... |%d| V"   , pipVals.busVoltage); 
  debugV("battery Voltage:...... |%s| V"   , String(pipVals.batteryVoltage).c_str());
  debugV("battery ChargeCurrent: |%d| A"   , pipVals.batteryChargeCurrent); 
  debugV("battery Charge:....... |%d| %"   , pipVals.batteryCharge); 
  debugV("inverter Temperature:. |%d| C"   , pipVals.inverterTemperature); 
  debugV("PV Current:........... |%s| A"   , String(pipVals.PVCurrent).c_str());
  debugV("PV Voltage:........... |%d| V"   , pipVals.PVVoltage); 
  debugV("PV Power:............. |%d| W"   , pipVals.PVPower);  
  debugV("Battery SCC:.......... |%d| V"   , pipVals.batterySCC);
  debugV("Batt DischargeCurrent: |%d| A"   , pipVals.batteryDischargeCurrent); 
  debugV("DeviceStatus:......... |%s|"     , pipVals.deviceStatus);
  debugV("Battery offset Fan:... |%d| V"   , pipVals.batOffsetFan);
  debugV("EEPROM Version:....... |%d|"     , pipVals.eepromVers);
  debugV("PV1 Charger Power:.... |%d| W"   , pipVals.PV1_chargPower);
  debugV("DeviceStatus2:........ |%s|"     , pipVals.deviceStatus2);
  }
}
/*
void inverter_LCD_data(String cmd)
{
  if (cmd==QPIGS)
  {
  // Print out QPIGS values on LCD
  lcdsetCursor(3,0); lcdprint("   ");   lcdsetCursor(3,0);  lcdprint(pipVals.gridVoltage);
  lcdsetCursor(8,0); lcdprint("  ");    lcdsetCursor(8,0);  lcdprint(pipVals.gridFrequency/10,0);
  lcdsetCursor(14,0); lcdprint("    "); lcdsetCursor(14,0); lcdprint(pipVals.acApparentPower);
  lcdsetCursor(3,1); lcdprint("   ");   lcdsetCursor(3,1);  lcdprint(pipVals.acOutput);
  lcdsetCursor(8,1); lcdprint("  ");    lcdsetCursor(8,1);  lcdprint(pipVals.acFrequency/10,0);
  lcdsetCursor(14,1); lcdprint("    "); lcdsetCursor(14,1); lcdprint(pipVals.acActivePower);
        
  // lcdprint("bus Voltage: "); lcdprint(pipVals.busVoltage/100); lcdprint(" V");    // not ift onto LCD
      
  lcdsetCursor(3,2); lcdprint("    ");  lcdsetCursor(3,2);  lcdprint(pipVals.batteryVoltage/100.00,2);
  lcdsetCursor(10,2); lcdprint("   ");  lcdsetCursor(10,2); lcdprint(pipVals.batteryChargeCurrent);
  lcdsetCursor(16,2); lcdprint("   ");  lcdsetCursor(16,2); lcdprint(pipVals.batteryCharge);
        
  // lcdprint("inverter Temperature:. ");  lcdprint(pipVals.inverterTemperature); lcdprint(" C");   // not fit onto LCD
        
  lcdsetCursor(3,3); lcdprint("  ");   lcdsetCursor(3,3);  lcdprint(pipVals.PVVoltage);
  lcdsetCursor(8,3); lcdprint("  ");   lcdsetCursor(8,3);  lcdprint(pipVals.PVCurrent);
  lcdsetCursor(12,3); lcdprint("   "); lcdsetCursor(12,3); lcdprint(pipVals.PVPower);
  lcdsetCursor(16,3); lcdprint("   "); lcdsetCursor(16,3); lcdprint(pipVals.loadPercent);
  LCDbase=true;  
  }
}

void inverter_LCD_base(String cmd)
{

  if (cmd=QPIGS)
  {
  lcdsetCursor(0,0);   lcdprint("Gr");  // Abbreviation of Grid
  lcdsetCursor(0,1);   lcdprint("Ou");  // Abbreviation of Output
  lcdsetCursor(0,2);   lcdprint("Ba");  // Abbreviation of Battery
  lcdsetCursor(0,3);   lcdprint("PV");  // Abbreviation of PhotoVoltaic
  //print metrics
  lcdsetCursor(6,0);   lcdprint("V");   // Grid Voltage
  lcdsetCursor(10,0);  lcdprint("Hz");  // Grid frequency
  lcdsetCursor(18,0);  lcdprint("VA");  // Output load Apparent power (VA)

  lcdsetCursor(6,1);   lcdprint("V");  //  Output Voltage
  lcdsetCursor(10,1);  lcdprint("Hz"); //  Output Frequency
  lcdsetCursor(19,1);  lcdprint("W");  //  Active power (load) 
  
  lcdsetCursor(8,2);   lcdprint("V");  // Battery Voltage
  lcdsetCursor(13,2);  lcdprint("A");  // Battery charge current
  lcdsetCursor(19,2);  lcdprint("%");  // Battery charge in %

  lcdsetCursor(5,3);   lcdprint("V");  // PV voltage
  lcdsetCursor(10,3);  lcdprint("A");  // PV current
  lcdsetCursor(15,3);  lcdprint("W");  // PV power
  lcdsetCursor(19,3);  lcdprint("%");  // Inverter load in %  
  }
}
*/

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
	Serial3.print(QPIGS+"\r");  //  QPIGS without CRC just for knock-knock for communiction exist
	Serial3.flush();          // Wait finishing transmitting before going on...
	if (Serial3.readStringUntil('\r') == "(NAKss" )   // check if get response for "knock-knock" from inverter on serial port.
	{
		uint16_t vgCrcCheck;
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

int INVERTER::invereter_receive( String cmd )
{
  if ( inverter_send(cmd)==0 )
    {
    // Serial.println("read data from inverter");
     inverterData = Serial3.readStringUntil('\x0D');

    // TEST for NAK
    // TEST for string lengh
    // TEST for CRC receipt match with calculated CRC
    // Different error hangling codes for each one
  
    debugV("Inverter anwser: |%s|", inverterData.substring(0,inverterData.length()-2));
    store_val(cmd);                // split inverter answer and store in pipVals
    inverter_console_data(cmd);     // print pipVals on serial port
    inverterData = "";             // empty string received by inverter
    return 0;
    }
  else
    {
 	  return -1;
	  }
    
}
