#include "inverter.h"


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

  val = strtok(0, " "); // Get the next value
  pipVals.gridFrequency = atof(val) * 10 ;

  val = strtok(0, " "); // Get the next value
  pipVals.acOutput = atoi(val);

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
  pipVals.PVCurrent = atoi(val);

  val = strtok(0, " "); // Get the next value
  pipVals.PVVoltage = atoi(val);

  pipVals.PVPower= pipVals.PVVoltage * pipVals.PVCurrent; // Calculate PV Power

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
  char val[8];
  strcpy(pipVals.deviceStatus, val);		// get the first value

  DevStatus2.changingFloatMode			 = val[0] ;		// 10: flag for charging to floating mode
  DevStatus2.SwitchOn				 = val[1] ;		// b9: Switch On
  DevStatus2.dustProof				     = val[2] ;		// b8: flag for dustproof installed(1-dustproof installed,0-no dustproof, only available for Axpert V series)
}

void INVERTER::inverter_console_data(String cmd)
{
  if (cmd==QPIGS)
  {
  // Print out readings
  Serial.print("grid Voltage:......... ");
  Serial.print(pipVals.gridVoltage); Serial.println(" V");
  Serial.print("grid Frequency:....... ");
  Serial.print(pipVals.gridFrequency/10.0,1); Serial.println(" Hz");
  Serial.print("AC Output:............ ");
  Serial.print(pipVals.acOutput); Serial.println(" V");
  Serial.print("AC Frequency:......... ");
  Serial.print(pipVals.acFrequency/10.0,1); Serial.println(" Hz");
  Serial.print("AC ApparentPower:..... ");
  Serial.print(pipVals.acApparentPower); Serial.println(" VA");
  Serial.print("AC ActivePower:....... ");
  Serial.print(pipVals.acActivePower); Serial.println(" W");
  Serial.print("load Percent:......... ");
  Serial.print(pipVals.loadPercent); Serial.println(" %");
  Serial.print("bus Voltage:.......... ");
  Serial.print(pipVals.busVoltage/100.00,2); Serial.println(" V"); 
  Serial.print("battery Voltage:...... ");
  Serial.print(pipVals.batteryVoltage/100.00,2); Serial.println(" V");
  Serial.print("battery ChargeCurrent: ");
  Serial.print(pipVals.batteryChargeCurrent); Serial.println(" A");
  Serial.print("battery Charge:....... ");
  Serial.print(pipVals.batteryCharge); Serial.println(" %");
  Serial.print("inverter Temperature:. ");
  Serial.print(pipVals.inverterTemperature/10.0); Serial.println(" C");
  Serial.print("PV Current:........... ");
  Serial.print(pipVals.PVCurrent); Serial.println(" A");
  Serial.print("PV Voltage:........... ");
  Serial.print(pipVals.PVVoltage); Serial.println(" V");
  Serial.print("PV Power:............. ");
  Serial.print(pipVals.PVPower);  Serial.println(" W");
  Serial.print("battery SCC:.......... ");
  Serial.print(pipVals.batterySCC/100.00,2); Serial.println(" V");
  Serial.print("battery DischargeCurrent: ");
  Serial.print(pipVals.batteryDischargeCurrent); Serial.println(" A");
  Serial.print("DeviceStatus:......... ");
  Serial.println(String(pipVals.deviceStatus));
  Serial.print("Battery offset Fan:... ");
  Serial.println(String(pipVals.batOffsetFan));
  Serial.print("EEPROM Version:....... ");
  Serial.println(String(pipVals.eepromVers));
  Serial.print("PV1 Charger Power:.... ");
  Serial.println(String(pipVals.PV1_chargPower));
  Serial.print("DeviceStatus2:........ ");
  Serial.println(String(pipVals.deviceStatus2));
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

String INVERTER::inverter_send(String inv_command)
{
	Serial3.print(NAK+"\r");  //  NAK-NAK ... knock-knock for communiction exist
	Serial3.flush();          // Wait finishing transmitting before going on...
	if ((Serial3.readStringUntil('\x0D')) == NAK )   // check if get response for "knock-knock" from inverter on serial port.
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
		return "-1";
   }
   return "";
}

void INVERTER::invereter_receive( String cmd )
{
  if ( inverter_send(cmd)!="-1" )
    {
/*    if (!LCDbase)
      {
      
      inverter_LCD_base(cmd);
      }
    if (nocommunication) 
      {
      nocommunication=false;
      }
 */ 
    // Serial.println("read data from inverter");
     inverterData = Serial3.readStringUntil('\x0D');
  
    Serial.println (inverterData.substring(0,inverterData.length()-2));
    store_val(cmd);                // split inverter answer and store in pipVals
    inverter_console_data(cmd);     // print pipVals on serial port
//    inverter_LCD_data(cmd);    // print (some) pipVals in LCD                
    inverterData = "";             // empty string received by inverter
    }
  else
    {
	  
	  Serial.println("No Inverter...");
/*    if (!nocommunication)
      {
      lcdclear();
      lcdsetCursor(4,0);
      lcdprint("No inverter");
      lcdsetCursor(3,1);
      lcdprint("communnication");
      nocommunication=true;
      LCDbase=false;
      delay(1000);
*/
	  }
    
}
