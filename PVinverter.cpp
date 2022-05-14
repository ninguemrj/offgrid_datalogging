/**********************************************************************************************************************
 * 
 * Arduino thread about communicating with the solar PV_INVERTER: https://forum.arduino.cc/t/rs232-read-data-from-mpp-solar-PV_INVERTER/600960/72
 * Thanks for "athersaleem", "DanX3", "sibianul", "Larryl79" and my other that shared their knowledge and finds in the Arduino forum.
 * Version 1.0 written by Larryl79 (https://github.com/larryl79/PV-Inverter-Communicator)
 * Version 2.0 Copied and changed to fit the Ricardo Jr´s project (https://github.com/ninguemrj/offgrid_datalogging/)
 * 
 * Contributors: Larryl79, Richardo Jr ( NiguemJr )
 * 
 ************************************************************************************************************************/

#include "PVinverter.h"

void PV_INVERTER::begin(uint32_t _baudRate, int _inverter_protocol, uint8_t _verbose_begin, int _timeout ) // "A" = 18 fields from QPIGS / "B" = 22 fields from QPIGS
{
  if (hwStream)
  {
    hwStream->begin(_baudRate);
    hwStream->setTimeout(_timeout);
  }
  else
  {
    swStream->begin(_baudRate);
    swStream->setTimeout(_timeout);
  }
  _streamRef = !hwStream? (Stream*)swStream : hwStream;

  // Initialize POP control status flag
  _POP_status = "";

  //--- prepare counter for smoothing reads (average)
  _average_count = 0;

  //--- For benchmarking the Solar PV_INVERTER communication ---------------
  _average_oldtime=millis();

  _VERBOSE_MODE = _verbose_begin;

  this->setProtocol(_inverter_protocol);

  // Initialize the pipvals and QPIGS_average with zeros (before first read)
  this->clear_QPIGS(QPIGS_values);
  this->clear_QPIGS(QPIGS_average);
  this->clear_QPIRI();


}

void PV_INVERTER::ESPyield()
{
  #if defined (ESP8266) || (defined ESP32)
  yield();
  #endif
}

/*--- clear_QPIRI() ----------------------------------------------------------------------
* 
* Clears infromation from QPIRI structure.
* Used when the information read from inverter isn't ok, avoiding storing bad data
------------------------------------------------------------------------------------------*/
void PV_INVERTER::clear_QPIRI()
{
  //3 = debug msg
  SUPPORT_FUNCTIONS::logMsg(3,"PVInverter: clear_QPIRI: FIRST line");
  
  this->QPIRI_values.GridRatingVoltage              = 0;
  this->QPIRI_values.GridRatingCurrent              = 0;
  this->QPIRI_values.ACOutputRatingVoltage          = 0;
  this->QPIRI_values.ACOutputRatingFrequency        = 0;
  this->QPIRI_values.ACOutputRatingCurrent          = 0;
  this->QPIRI_values.ACOutputRatingApparentPower    = 0;
  this->QPIRI_values.ACOutputRatingActivePower      = 0;
  this->QPIRI_values.BatteryRatingVoltage           = 0;
  this->QPIRI_values.BatteryReChargeVoltage         = 0;   
  this->QPIRI_values.BatteryUnderVoltage            = 0;          
  this->QPIRI_values.BatteryBulkVoltage             = 0;     
  this->QPIRI_values.BatteryFloatVoltage            = 0;     
  this->QPIRI_values.BatteryType                    = 0;                    
  this->QPIRI_values.MaxAC_ChargingCurrent          = 0;     
  this->QPIRI_values.MaxChargingCurrent             = 0;     
  this->QPIRI_values.InputVoltageRange              = 0;     
  this->QPIRI_values.OutputSourcePriority           = 0;                
  this->QPIRI_values.ChargerSourcePriority          = 0;          
}


/*--- store_QPIRI() ----------------------------------------------------------------------
* 
* Pupolates QPIRI structure with data received from inverter.
*------------------------------------------------------------------------------------------*/
void PV_INVERTER::store_QPIRI(String _value, uint32_t _now)
{
  //3 = debug msg
  SUPPORT_FUNCTIONS::logMsg(3,"PVInverter: store_QPIRI: FIRST: _value" + _value + "| _now" + _now);

  if (_value == "")
  {
    //--- QPIRI without data, skip this reading and wait next one ----------------- 
    this->clear_QPIRI(); 
    QPIGS_values.bat_backToUtilityVolts = 0;
    QPIGS_values.bat_bulkChargeVolts    = 0;
    QPIGS_values.bat_FloatChargeVolts   = 0;
    QPIGS_values.bat_CutOffVolts        = 0;
    QPIGS_values.OutPutPriority         = 0;
    QPIGS_values.ChargerSourcePriority  = 0;  
  }
  else
  {
     // Sets provided unixtime as argument for the QPIRI_values
    this->QPIRI_values._unixtime = _now;

    //--- Update status with data from PV_INVERTER  ---------------------------------  
    char pipInputBuf[200];
    char *val;
    
    strcpy(pipInputBuf, _value.c_str());
    
    //--- Now split the packet into the values ------------------------------------
    val = strtok((char *) pipInputBuf, " ");                 // B = (xxx.x V * 10
    this->QPIRI_values.GridRatingVoltage  = atof(val+1)*10;

    val = strtok(0, " ");                                    // C = xx.x A * 10
    this->QPIRI_values.GridRatingCurrent  = atof(val) * 10 ;

    val = strtok(0, " ");                                    // D = xxx.x V * 10
    this->QPIRI_values.ACOutputRatingVoltage  = atof(val) * 10 ;

    val = strtok(0, " ");                                    // E = xx.x Hz * 10
    this->QPIRI_values.ACOutputRatingFrequency  = atof(val) * 10 ;

    val = strtok(0, " ");                                    // F = xx.x A * 10
    this->QPIRI_values.ACOutputRatingCurrent  = atof(val) * 10 ;

    val = strtok(0, " ");                                    // H = xxxx VA
    this->QPIRI_values.ACOutputRatingApparentPower  = atoi(val) ;

    val = strtok(0, " ");                                    // I = xxxx W
    this->QPIRI_values.ACOutputRatingActivePower  = atoi(val) ;

    val = strtok(0, " ");                                    // J = xx.x V * 10
    this->QPIRI_values.BatteryRatingVoltage  = atof(val) * 10 ;

    val = strtok(0, " ");                                    // K = xx.x V * 10 (battery re-charge or backToUtilityVolts)
    QPIGS_values.bat_backToUtilityVolts = atof(val) * 10 ;
    this->QPIRI_values.BatteryReChargeVoltage = atof(val) * 10 ; // doubled to not beak legancy code using QPIGS_Values (need to be removed)
    
    val = strtok(0, " ");                                    // L = xx.x V * 10 bat_CutOffVolts
    QPIGS_values.bat_CutOffVolts = atof(val) * 10 ;
    this->QPIRI_values.BatteryUnderVoltage = atof(val) * 10; // doubled to not beak legancy code using QPIGS_Values (need to be removed)
    
    val = strtok(0, " ");                                    // M = xx.x V * 10 bat_bulkChargeVolts
    QPIGS_values.bat_bulkChargeVolts = atof(val) * 10 ;
    this->QPIRI_values.BatteryBulkVoltage = atof(val) * 10 ; // doubled to not beak legancy code using QPIGS_Values (need to be removed)
    
    val = strtok(0, " ");                                    // N = xx.x V * 10 bat_FloatChargeVolts
    QPIGS_values.bat_FloatChargeVolts = atof(val) * 10 ;  
    this->QPIRI_values.BatteryFloatVoltage = atof(val) * 10; // doubled to not beak legancy code using QPIGS_Values (need to be removed)
    
    val = strtok(0, " ");                                    // O = 0: AGM 1: Flooded 2: User 3: Pylon 5: Weco 6: Soltaro 8: Lib 9: Lic
    this->QPIRI_values.BatteryType = atoi(val);                    
    
    val = strtok(0, " ");                                    // P = xxx A
    this->QPIRI_values.MaxAC_ChargingCurrent = atoi(val);     
    
    val = strtok(0, " ");                                    // Q = xx0 A
    this->QPIRI_values.MaxChargingCurrent = atoi(val);     
    
    val = strtok(0, " ");                                    // O = 0: Appliance 1: UPS
    this->QPIRI_values.InputVoltageRange = atoi(val);     
    
    val = strtok(0, " ");                                    // P = 0: UtilitySolarBat 1: SolarUtilityBat 2: SolarBatUtility -> OutPutPriority 
    QPIGS_values.OutPutPriority  = atoi(val);
    this->QPIRI_values.OutputSourcePriority  = atoi(val);    // doubled to not beak legancy code using QPIGS_Values (need to be removed)
    
    val = strtok(0, " ");                                    // Q = 0: Utility first 1: Solar first 2: Solar + Utility 3: Only solar charging permitted ( protocol 2 1-3 ) -> ChargerSourcePriority
    QPIGS_values.ChargerSourcePriority  = atoi(val);   
    this->QPIRI_values.ChargerSourcePriority  = atoi(val);   // doubled to not beak legancy code using QPIGS_Values (need to be removed)  

   // ignore the other QPIRI fields for while
  }

}


String PV_INVERTER::debug_QPIRI()
{
  //3 = debug msg
  SUPPORT_FUNCTIONS::logMsg(3,"PVInverter: debug_QPIRI: First line");

  String _response = String("\n\rUNIX TIME:..................: ") + String(this->QPIRI_values._unixtime - __FUSO__) + " Seconds\n\r" +
  // B = xx.x V * 10
  "GridRatingVoltage...........: " + String(this->QPIRI_values.GridRatingVoltage / 10.0 ) + " V\n\r" +
  
  // C = xx.x A * 10
  "GridRatingCurrent...........: " + String(this->QPIRI_values.GridRatingCurrent / 10.0 ) + " A\n\r" +

  // D = xxx.x V * 10
  "ACOutputRatingVoltage.......: " + String(this->QPIRI_values.ACOutputRatingVoltage / 10.0 ) + " V\n\r" +

  // E = xx.x Hz * 10
  "ACOutputRatingFrequency.....: " + String(this->QPIRI_values.ACOutputRatingFrequency / 10.0 ) + " Hz\n\r" +

  // F = xx.x A * 10
  "ACOutputRatingCurrent.......: " + String(this->QPIRI_values.ACOutputRatingCurrent / 10.0 ) + " A\n\r" +

  // H = xxxx VA
  "ACOutputRatingApparentPower.: " + String(this->QPIRI_values.ACOutputRatingApparentPower) + " VA\n\r" +

  // I = xxxx W
  "ACOutputRatingActivePower...: " + String(this->QPIRI_values.ACOutputRatingActivePower) + " W\n\r" +

  // J = xx.x V * 10
  "BatteryRatingVoltage........: " + String(this->QPIRI_values.BatteryRatingVoltage / 10.0 ) + " V\n\r" +

  // K = xx.x V * 10 (battery re-charge or backToUtilityVolts)
  "BatteryReChargeVoltage......: " + String(this->QPIRI_values.BatteryReChargeVoltage/ 10.0 ) + " V (backToUtilityVolts)\n\r" + 
    
  // L = xx.x V * 10 bat_CutOffVolts
  "BatteryUnderVoltage.........: " + String(this->QPIRI_values.BatteryUnderVoltage/ 10.0) + " V (bat_CutOffVolts)\n\r" + 
    
  // M = xx.x V * 10 bat_bulkChargeVolts
  "BatteryBulkVoltage..........: " + String(this->QPIRI_values.BatteryBulkVoltage/ 10.0 ) + " V\n\r" + 
    
  // N = xx.x V * 10 bat_FloatChargeVolts
  "BatteryFloatVoltage.........: " + String(this->QPIRI_values.BatteryFloatVoltage/ 10.0) + " V\n\r" + 
    
  // O = 0: AGM 1: Flooded 2: User 3: Pylon 5: Weco 6: Soltaro 8: Lib 9: Lic
  "BatteryType.................: " + String(this->QPIRI_values.BatteryType) + " | 0: AGM 1: Flooded 2: User 3: Pylon 5: Weco 6: Soltaro 8: Lib 9: Lic\n\r" +                    
    
  // P = xxx A
  "MaxAC_ChargingCurrent.......: " + String(this->QPIRI_values.MaxAC_ChargingCurrent) + " A\n\r" +     
    
  // Q = xx0 A
  "MaxChargingCurrent..........: " + String(this->QPIRI_values.MaxChargingCurrent) + " A\n\r" +     
    
  // O = 0: Appliance 1: UPS
  "InputVoltageRange...........: " + String(this->QPIRI_values.InputVoltageRange) + " | 0: Appliance 1: UPS\n\r" +     
    
  // P = 0: UtilitySolarBat 1: SolarUtilityBat 2: SolarBatUtility -> OutPutPriority 
  "OutputSourcePriority........: " + String(this->QPIRI_values.OutputSourcePriority) + " | 0: UtilitySolarBat 1: SolarUtilityBat 2: SolarBatUtility\n\r" +    
    
  // Q = 0: Utility first 1: Solar first 2: Solar + Utility 3: Only solar charging permitted ( protocol 2 1-3 ) -> ChargerSourcePriority
  "ChargerSourcePriority.......: " + String(this->QPIRI_values.ChargerSourcePriority) + " | 0: Utility first 1: Solar first 2: Solar + Utility 3: Only solar charging permitted\n\r"; 

   // ignore the other QPIRI fields for while

   return _response;
}



/*--- store_QPIGS() ----------------------------------------------------------------------
* 
* Pupolates QPIGS structure with data received from inverter.
*------------------------------------------------------------------------------------------*/
void PV_INVERTER::store_QPIGS(String _value, uint32_t _now)
{

  if (_value=="")
  {
    // Clear pipvals in case of an incomplete or no reading
    this->clear_QPIGS(QPIGS_average);

        
  } 
  else 
  {
    
    // Sets provided unixtime as argument for the QPIGS_values
    QPIGS_values._unixtime = _now;
    
    //--- Update status with data read from inverter serial communication ---------------------------------  
    char pipInputBuf[500];
    char *val;
    
    strcpy(pipInputBuf, _value.c_str());
    
    //--- Now split the packet into the values ------------------------------------
    val = strtok((char *) pipInputBuf, " "); // get the first value
    if (atof(val + 1) >10)   // aviod false value stored, because it shows 2-3V even if grid isn't connected.
    {
      QPIGS_values.gridVoltage = atoi(val + 1);  // Skip the initial '('
    }
    else
    {
      QPIGS_values.gridVoltage = 0;
    }
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.gridFrequency = atof(val) * 10 ;
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.acOutput = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.acFrequency = atof(val) * 10;
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.acApparentPower = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.acActivePower = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.loadPercent = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.busVoltage = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.batteryVoltage = atof(val)*100;
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.batteryChargeCurrent = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.batteryCharge = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.inverterTemperature = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.PVCurrent = atof(val)*10;
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.PVVoltage = atof(val)*10;
    
    QPIGS_values.PVPower = QPIGS_values.PVVoltage * QPIGS_values.PVCurrent; // Calculate PV Power
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.batterySCC = atof(val)*100;
    
    val = strtok(0, " "); // Get the next value
    QPIGS_values.batteryDischargeCurrent = atoi(val);
    
    val = strtok(0, " "); // Get the next value
    char ds_temp[9];
    strcpy(ds_temp, val);
    QPIGS_values.DevStat_SBUpriority     = ds_temp[0]-'0';
    QPIGS_values.DevStat_ConfigStatus    = ds_temp[1]-'0';
    QPIGS_values.DevStat_FwUpdate        = ds_temp[2]-'0'; 
    QPIGS_values.DevStat_LoadStatus      = ds_temp[3]-'0'; 
    QPIGS_values.DevStat_BattVoltSteady  = ds_temp[4]-'0'; 
    QPIGS_values.DevStat_Chargingstatus  = ds_temp[5]-'0';  
    QPIGS_values.DevStat_SCCcharge       = ds_temp[6]-'0'; 
    QPIGS_values.DevStat_ACcharge        = ds_temp[7]-'0'; 
    
    if ( this->getProtocol() == 2)   // 2 = 22 fields from QPIGS
    {
      val = strtok(0, " "); // Get the next value
      QPIGS_values.batOffsetFan = atoi(val);
    
      val = strtok(0, " "); // Get the next value
      QPIGS_values.eepromVers = atoi(val);
    
      val = strtok(0, " "); // Get the next value
      QPIGS_values.PV1_chargPower = atoi(val);
    
      val = strtok(0, " "); // Get the next value
      strcpy(ds_temp, val);
      QPIGS_values.DevStat_chargingFloatMode = ds_temp[0]-'0'; 
      QPIGS_values.DevStat_SwitchOn          = ds_temp[1]-'0'; 
      QPIGS_values.DevStat_dustProof         = ds_temp[2]-'0'; 
    }
  }
}

//--- When _average_count is from 0 to 9, this function Accumulates direct readings from pipVals into _QPIGS_tempAverage structure;
//--- When _average_count is = 9, additionally it averages the accumulated readings and updates the public QPIGS_average structure with the smoothed values
void PV_INVERTER::smoothing_QPIGS()
{
  if ((this->QPIGS_values.gridVoltage == 0) && (this->QPIGS_values.acOutput == 0) && (this->QPIGS_values.batteryVoltage == 0) && (this->QPIGS_values.PVVoltage == 0))
  {
    //--- QPIGS without data, skip this reading and wait next one -----------------  
     _average_count--;  
  }
  else
  {
    // Accumulaets readings on temp structure
    _QPIGS_tempAverage._unixtime                =  QPIGS_values._unixtime;         // take the lastest read string only
    _QPIGS_tempAverage.gridVoltage              += QPIGS_values.gridVoltage;
    _QPIGS_tempAverage.gridFrequency            += QPIGS_values.gridFrequency;
    _QPIGS_tempAverage.acOutput                 += QPIGS_values.acOutput;
    _QPIGS_tempAverage.acFrequency              += QPIGS_values.acFrequency;
    _QPIGS_tempAverage.acApparentPower          += QPIGS_values.acApparentPower;
    _QPIGS_tempAverage.acActivePower            += QPIGS_values.acActivePower;
    _QPIGS_tempAverage.loadPercent              += QPIGS_values.loadPercent;
    _QPIGS_tempAverage.busVoltage               += QPIGS_values.busVoltage;
    _QPIGS_tempAverage.batteryVoltage           += QPIGS_values.batteryVoltage;
    _QPIGS_tempAverage.batteryChargeCurrent     += QPIGS_values.batteryChargeCurrent;
    _QPIGS_tempAverage.batteryCharge            += QPIGS_values.batteryCharge;
    _QPIGS_tempAverage.inverterTemperature      += QPIGS_values.inverterTemperature;
    _QPIGS_tempAverage.PVCurrent                += QPIGS_values.PVCurrent;
    _QPIGS_tempAverage.PVVoltage                += QPIGS_values.PVVoltage;
    _QPIGS_tempAverage.PVPower                  += QPIGS_values.PVPower;
    _QPIGS_tempAverage.batterySCC               += QPIGS_values.batterySCC;
    _QPIGS_tempAverage.batteryDischargeCurrent  += QPIGS_values.batteryDischargeCurrent;
    _QPIGS_tempAverage.DevStat_SBUpriority      =  QPIGS_values.DevStat_SBUpriority;        // take the lastest read string only
    _QPIGS_tempAverage.DevStat_ConfigStatus     =  QPIGS_values.DevStat_ConfigStatus;       // take the lastest read string only
    _QPIGS_tempAverage.DevStat_FwUpdate         =  QPIGS_values.DevStat_FwUpdate;           // take the lastest read string only
    _QPIGS_tempAverage.DevStat_LoadStatus       =  QPIGS_values.DevStat_LoadStatus;         // take the lastest read string only
    _QPIGS_tempAverage.DevStat_BattVoltSteady   =  QPIGS_values.DevStat_BattVoltSteady;     // take the lastest read string only
    _QPIGS_tempAverage.DevStat_Chargingstatus   =  QPIGS_values.DevStat_Chargingstatus;     // take the lastest read string only
    _QPIGS_tempAverage.DevStat_SCCcharge        =  QPIGS_values.DevStat_SCCcharge;          // take the lastest read string only
    _QPIGS_tempAverage.DevStat_ACcharge         =  QPIGS_values.DevStat_ACcharge;           // take the lastest read string only
    
    if ( this->getProtocol() == 2)   // 2 = 22 fields from QPIGS
    {
      _QPIGS_tempAverage.PV1_chargPower           += QPIGS_values.PV1_chargPower;
      _QPIGS_tempAverage.batOffsetFan             =  QPIGS_values.batOffsetFan;              // take the lastest read string only
      _QPIGS_tempAverage.eepromVers               =  QPIGS_values.eepromVers;                // take the lastest read string only
      _QPIGS_tempAverage.DevStat_chargingFloatMode=  QPIGS_values.DevStat_chargingFloatMode; // take the lastest read string only
      _QPIGS_tempAverage.DevStat_SwitchOn         =  QPIGS_values.DevStat_SwitchOn;          // take the lastest read string only
      _QPIGS_tempAverage.DevStat_dustProof        =  QPIGS_values.DevStat_dustProof;         // take the lastest read string only
    }

    //--- when _average_count = 9: calculate average amounts to update QPIGS_average structure ---------------
    if(_average_count == 9)
    {
        QPIGS_average._unixtime               = _QPIGS_tempAverage._unixtime;         // take the lastest read string only
        QPIGS_average.gridVoltage             = _QPIGS_tempAverage.gridVoltage/10;
        QPIGS_average.gridFrequency           = _QPIGS_tempAverage.gridFrequency/10 ;
        QPIGS_average.acOutput                = _QPIGS_tempAverage.acOutput/10;
        QPIGS_average.acFrequency             = _QPIGS_tempAverage.acFrequency/10;
        QPIGS_average.acApparentPower         = _QPIGS_tempAverage.acApparentPower/10;
        QPIGS_average.acActivePower           = _QPIGS_tempAverage.acActivePower/10;
        QPIGS_average.loadPercent             = _QPIGS_tempAverage.loadPercent/10;
        QPIGS_average.busVoltage              = _QPIGS_tempAverage.busVoltage/10;
        QPIGS_average.batteryVoltage          = _QPIGS_tempAverage.batteryVoltage/10;
        QPIGS_average.batteryChargeCurrent    = _QPIGS_tempAverage.batteryChargeCurrent/10;
        QPIGS_average.batteryCharge           = _QPIGS_tempAverage.batteryCharge/10;
        QPIGS_average.inverterTemperature     = _QPIGS_tempAverage.inverterTemperature/10;
        QPIGS_average.PVCurrent               = _QPIGS_tempAverage.PVCurrent/10;
        QPIGS_average.PVVoltage               = _QPIGS_tempAverage.PVVoltage/10;
        QPIGS_average.PVPower                 = _QPIGS_tempAverage.PVPower/10;
        QPIGS_average.batterySCC              = _QPIGS_tempAverage.batterySCC /10;
        QPIGS_average.batteryDischargeCurrent = _QPIGS_tempAverage.batteryDischargeCurrent/10;
        QPIGS_average.DevStat_SBUpriority     = _QPIGS_tempAverage.DevStat_SBUpriority;        // take the lastest read string only
        QPIGS_average.DevStat_ConfigStatus    = _QPIGS_tempAverage.DevStat_ConfigStatus;       // take the lastest read string only
        QPIGS_average.DevStat_FwUpdate        = _QPIGS_tempAverage.DevStat_FwUpdate;           // take the lastest read string only
        QPIGS_average.DevStat_LoadStatus      = _QPIGS_tempAverage.DevStat_LoadStatus;         // take the lastest read string only
        QPIGS_average.DevStat_BattVoltSteady  = _QPIGS_tempAverage.DevStat_BattVoltSteady;     // take the lastest read string only
        QPIGS_average.DevStat_Chargingstatus  = _QPIGS_tempAverage.DevStat_Chargingstatus;     // take the lastest read string only
        QPIGS_average.DevStat_SCCcharge       = _QPIGS_tempAverage.DevStat_SCCcharge;          // take the lastest read string only
        QPIGS_average.DevStat_ACcharge        = _QPIGS_tempAverage.DevStat_ACcharge;           // take the lastest read string only
        
        if ( this->getProtocol() == 2 )   // 2 = 22 fields from QPIGS
        {
          QPIGS_average.PV1_chargPower           = _QPIGS_tempAverage.PV1_chargPower/10;
          QPIGS_average.batOffsetFan             = _QPIGS_tempAverage.batOffsetFan;  // take the lastest read string
          QPIGS_average.eepromVers               = _QPIGS_tempAverage.eepromVers;    // take the lastest read string
          QPIGS_average.DevStat_chargingFloatMode= _QPIGS_tempAverage.DevStat_chargingFloatMode; 
          QPIGS_average.DevStat_SwitchOn         = _QPIGS_tempAverage.DevStat_SwitchOn; 
          QPIGS_average.DevStat_dustProof        = _QPIGS_tempAverage.DevStat_dustProof; 
        }

        //--- RESETs the _QPIGS_tempAverage values to not accummulate the next 10 readings with previous ones ----
        this->clear_QPIGS(_QPIGS_tempAverage);

        //--- RESETs average counting -----------------------------------------------------
        _average_count = 0;  
    }
    else
    {
      //--- Prepare counting for next array posotion -------------------------------
      _average_count++;
    }
  }
}

/*--- clear_QPIGS() ----------------------------------------------------------------------
* 
* Clears infromation from QPIRI structure.
* Used when the information read from inverter isn't ok, avoiding storing bad data
------------------------------------------------------------------------------------------*/
void PV_INVERTER::clear_QPIGS (pipVals_t &_thisPIP)
{
    _thisPIP.gridVoltage              = 0;
    _thisPIP.gridFrequency            = 0;
    _thisPIP.acOutput                 = 0;
    _thisPIP.acFrequency              = 0;
    _thisPIP.acApparentPower          = 0;
    _thisPIP.acActivePower            = 0;
    _thisPIP.loadPercent              = 0;
    _thisPIP.busVoltage               = 0;
    _thisPIP.batteryVoltage           = 0;
    _thisPIP.batteryChargeCurrent     = 0;
    _thisPIP.batteryCharge            = 0;
    _thisPIP.inverterTemperature      = 0;
    _thisPIP.PVCurrent                = 0;
    _thisPIP.PVVoltage                = 0;
    _thisPIP.PVPower                  = 0;
    _thisPIP.batterySCC               = 0;
    _thisPIP.batteryDischargeCurrent  = 0;
    _thisPIP.DevStat_SBUpriority      = 0;
    _thisPIP.DevStat_ConfigStatus     = 0;
    _thisPIP.DevStat_FwUpdate         = 0;
    _thisPIP.DevStat_LoadStatus       = 0;
    _thisPIP.DevStat_BattVoltSteady   = 0;
    _thisPIP.DevStat_Chargingstatus   = 0;
    _thisPIP.DevStat_SCCcharge        = 0;
    _thisPIP.DevStat_ACcharge         = 0;
 
     if ( this->getProtocol() == 2 )   // 2 = 22 fields from QPIGS
    {
      _thisPIP.PV1_chargPower           = 0;
      _thisPIP.batOffsetFan             = 0;  
      _thisPIP.eepromVers               = 0;    
      _thisPIP.PV1_chargPower           = 0;
      _thisPIP.DevStat_chargingFloatMode= 0; 
      _thisPIP.DevStat_SwitchOn         = 0; 
      _thisPIP.DevStat_dustProof        = 0;
    }
}

String PV_INVERTER::debug_QPIGS(pipVals_t _thisPIP)
{
  String _response = String("\n\rUNIX TIME:................ ") + String(_thisPIP._unixtime - __FUSO__) + " Seconds\n\r" +
  "Grid Voltage:............. " + String(_thisPIP.gridVoltage) + " V\n\r"             +
  "Grid Frequency:........... " + String(_thisPIP.gridFrequency/10.0) + " Hz\n\r"     +
  "AC Output:................ " + String(_thisPIP.acOutput) + " V\n\r"                +
  "AC Frequency:............. " + String(_thisPIP.acFrequency/10.0) + " Hz\n\r"       +
  "AC ApparentPower:......... " + String(_thisPIP.acApparentPower) + " VA\n\r"        +
  "AC ActivePower:........... " + String(_thisPIP.acActivePower) + " W\n\r"           +
  "Load Percent:............. " + String(_thisPIP.loadPercent) + " %\n\r"             +
  "Bus Voltage:.............. " + String(_thisPIP.busVoltage) + " V\n\r"              + 
  "Battery Voltage:.......... " + String(_thisPIP.batteryVoltage/100.00)+ " V\n\r"    +
  "Battery ChargeCurrent:.... " + String(_thisPIP.batteryChargeCurrent) + " A\n\r"    + 
  "Battery Charge:........... " + String(_thisPIP.batteryCharge) + " %\n\r"           + 
  "PV_INVERTER Temperature:.. " + String(_thisPIP.inverterTemperature) + "C\n\r"      + 
  "PV Current:............... " + String(_thisPIP.PVCurrent /10.0)+ " A\n\r"          +
  "PV Voltage:............... " + String(_thisPIP.PVVoltage /10.0) + " V\n\r"         + 
  "PV Power:................. " + String(_thisPIP.PVPower   /100.00) + " W\n\r"       +  
  "Battery SCC:.............. " + String(_thisPIP.batterySCC/100.00) + " V\n\r"       + 
  "Batt DischargeCurrent:.... " + String(_thisPIP.batteryDischargeCurrent) + " A\n\r" + 
  "DevStat_SBUpriority:...... " + String(_thisPIP.DevStat_SBUpriority) + " \n\r"      + 
  "DevStat_ConfigStatus:..... " + String(_thisPIP.DevStat_ConfigStatus) + " \n\r"     + 
  "DevStat_FwUpdate:......... " + String(_thisPIP.DevStat_FwUpdate) + " \n\r"         + 
  "DevStat_LoadStatus:....... " + String(_thisPIP.DevStat_LoadStatus) + " \n\r"       + 
  "DevStat_BattVoltSteady:... " + String(_thisPIP.DevStat_BattVoltSteady) + " \n\r"   + 
  "DevStat_Chargingstatus:... " + String(_thisPIP.DevStat_Chargingstatus) + " \n\r"   + 
  "DevStat_SCCcharge:........ " + String(_thisPIP.DevStat_SCCcharge) + " \n\r"        + 
  "DevStat_ACcharge:......... " + String(_thisPIP.DevStat_ACcharge) + " \n\r"; 



  if ( this->getProtocol() == 2 )   // 2 = 22 fields from QPIGS
  {
    _response += 
    String("Battery offset Fan:....... ") + String(_thisPIP.batOffsetFan) + " V\n\r"   +
    "EEPROM Version:........... " + String(_thisPIP.eepromVers)           + "\n\r"     +
    "PV1 Charger Power:........ " + String(_thisPIP.PV1_chargPower) + " W\n\r"         +
    "DevStat_chargingFloatMode: " + String(_thisPIP.DevStat_chargingFloatMode)+ "\n\r" +
    "DevStat_SwitchOn:......... " + String(_thisPIP.DevStat_SwitchOn)+ "\n\r"          +
    "DevStat_dustProof:........ " + String(_thisPIP.DevStat_dustProof)+ "\n\r";
  }
  
  // QPIRI values: TODO: Move to a different function
  _response += 
  String("Bat Back to Grid:......... ") + String(_thisPIP.bat_backToUtilityVolts/10.0) + " V\n\r"   + 
  "Bat Bulk Charge:.......... " + String(_thisPIP.bat_bulkChargeVolts/10.0) + " V\n\r"              + 
  "Bat Float Charge:......... " + String(_thisPIP.bat_FloatChargeVolts/10.0) + " V\n\r"             + 
  "Bat CutOff:............... " + String(_thisPIP.bat_CutOffVolts/10.0) + " V\n\r"                  + 
  "Output Priority:.......... " + String(_thisPIP.OutPutPriority) + " | 0: Utility first / 1: Solar first / 2: SBU first\n\r"   + 
  "Charging Priority:........ " + String(_thisPIP.ChargerSourcePriority) + " | 0: Utility first / 1: Solar first / 2: Solar + Utility / 3: Only solar\n\r";

   return _response;
}


// ******************************************  PV_INVERTER communication  *********************************

bool PV_INVERTER::rap()   //knocknock to get synced commauncation if avail
{
  bool _communication = false;
  this->_streamRef->print("QKnock-Knock\x0D");  //  knock-knock for communiction exist
  this->_streamRef->flush();          // Wait finishing transmitting before going on...
  if (this->_streamRef->readStringUntil('\x0D') == "(NAKss" )   // check if get response for "knock-knock" from PV_INVERTER on serial port.
    { _communication = true; }
    return _communication; // true if ok, false for no communication.
}

char PV_INVERTER::read(char _cmd)   // new serial read function, no ready yet, and not tested.
{
  char _str_return;
   if ( this->send(String(_cmd)) == 0 )
    {
      while ( _streamRef->available() > 0)
        {
        _str_return = _streamRef->read();
        if ( _str_return == '\x0D' )
          {
            return _str_return;
          }
        this->ESPyield();
        }
    }
  return false;
}

int PV_INVERTER::send(String _inv_command, bool _CRChardcoded)
{
  if ( this->rap() )   // check if get response for "knock-knock" from PV_INVERTER on serial port.
  {
    if (!_CRChardcoded)
      {
        _inv_command += this->addCRC(_inv_command);   // add CRC sting to the command by protocol number
      }
    _inv_command += "\x0D";     // add CR
    //Sending Request to PV_INVERTER
    _streamRef->print(_inv_command);
    _streamRef->flush();          // Wait finishing transmitting before going on...
  }
  else
  {
    return 1; // No serial communication, error code 1
  }
   return 0; // NAKss returned, serial communication up and running
}

int PV_INVERTER::receive( String _cmd, String &str_return,  bool _CRChardcoded )
{
  if ( this->send(_cmd, _CRChardcoded) == 0 )
    {
      str_return = _streamRef->readStringUntil('\x0D');
      
      // checking Command not recognized 
      if (str_return == "(NAKss") 
      {
        // 2 = error msg
        SUPPORT_FUNCTIONS::logMsg(2, "PV_INVERTER: " + _cmd + ": Not recognized command: " + str_return);
        return 2;   
      }

      // TODO: TEST for CRC receipt match with calculated CRC
      
      // 3 = debug msg
      SUPPORT_FUNCTIONS::logMsg(3, "PV_INVERTER: " + _cmd + ": Command executed successfully. Returned: " + str_return);

      return 0;
    }
    else
    {
      // No serial communication
      // 2 = error msg
      SUPPORT_FUNCTIONS::logMsg(2, "PV_INVERTER: " + _cmd + ": No serial communication");
      str_return = "";
      return 1;
    }
    
}

void PV_INVERTER::ask_QPIRI( String &_result, bool _CRChardcoded)
  {
      int _funct_return = 0;
      _result = "";

      // 3 = debug msg
      SUPPORT_FUNCTIONS::logMsg(3, "PV_INVERTER: ask_QPIRI: before this->receive | _CRChardcoded = |" + String(_CRChardcoded) + "|.  Returned: " + _result);

      //workaround for diff CRC then calculated. Check a better way to do it later
      if (_CRChardcoded)
      {
        _funct_return = this->receive(QPIRICRC, _result, _CRChardcoded);
      }
      else
      {
        _funct_return = this->receive(QPIRI, _result);
      }
      // 3 = debug msg
      SUPPORT_FUNCTIONS::logMsg(3, "PV_INVERTER: ask_QPIRI: after this->receive | _CRChardcoded = |" + String(_CRChardcoded) + "|.  _funct_return: " + _funct_return + " | _result: " + _result);
      
      if (_funct_return == 0) 
      {
        // checking return string lengh for QPIRI command 
        if (strlen(_result.c_str()) < 85)       
        {
          // 2 = error msg
          SUPPORT_FUNCTIONS::logMsg(2, "PV_INVERTER: QPIRI: Receipt string is not completed, size = |" + String(strlen(_result.c_str())) + "|.  Returned: " + _result);
          _result = "";                                    // clear the string result from PV_INVERTER as it is not complete
          _funct_return = 1;                              // short string lengh for QPIRI command 
        }
      }
  }

int PV_INVERTER::ask_data(uint32_t _now,  bool _CRChardcoded)
    {
      int _funct_return = 0;
      String _result = "";
      
      //workaround for diff CRC then calculated. Check a better way to do it later
      if (_CRChardcoded)
      {
        _funct_return = this->receive(QPIGSCRC, _result, _CRChardcoded);
      }
      else
      {
        _funct_return = this->receive(QPIGS, _result, _CRChardcoded);
      } 
      if (_funct_return == 0) 
      {
        //debug msg
        SUPPORT_FUNCTIONS::logMsg(3, "PV_INVERTER: QPIGS: Receipt string size = |" + String(strlen(_result.c_str())) + "|.  Returned: " + _result);
        
        // checking return string lengh for QPIGS command 
        if (strlen(_result.c_str()) < 85)       
        {
          //2 = error msg
          SUPPORT_FUNCTIONS::logMsg(2, "PV_INVERTER: QPIGS: Receipt string is not completed, size = |" + String(strlen(_result.c_str())) + "|.  Returned: " + _result);
          _result = "";                                 // clear the string result from PV_INVERTER as it is not complete
          _funct_return = 1;                            // short string lengh for QPIGS command 
        }

        this->store_QPIGS(_result.c_str(), _now);       // Updates with direct reading from inverter
        //3 = debug msg
        SUPPORT_FUNCTIONS::logMsg(3,"After store_QPIGS");
        this->smoothing_QPIGS();                        // accumulates, average and updates QPIGS_average.
        //3 = debug msg
        SUPPORT_FUNCTIONS::logMsg(3,"After smoothing_QPIGS: _average_count: " + String(_average_count));

        
        // Ask Inverer for QPIRI configuration in the 10th QPIGS reading (0 to 9)
        // (when the averaged amount will be stored in the public variables)
        if (_average_count == 9)
        {
        
          // Reads QPIRI command to check battery configurations
          
          String _QPIRI_result;
          this->ask_QPIRI(_QPIRI_result, _CRChardcoded);
          //3 = debug msg
          SUPPORT_FUNCTIONS::logMsg(3,"After ask_QPIRI");
  
          // store QPIRI info
          this->store_QPIRI(_QPIRI_result, _now);
          //3 = debug msg
          SUPPORT_FUNCTIONS::logMsg(3,"After store_QPIRI");
  
          //--- For benchmarking the averaged Solar PV_INVERTER communication ---------------------------      
          // 4 = benchmark msg
          SUPPORT_FUNCTIONS::logMsg(4, "PV_INVERTER: ask_data: Time to read, acummulate and average QPIGS info: " + String((millis() - _average_oldtime)));
          
          // 3 = debug msg
          SUPPORT_FUNCTIONS::logMsg(3, this->debug_QPIRI()); 

          
          // prepare for a new banchmarck
          _average_oldtime = millis();
        }
        //3 = debug msg
        SUPPORT_FUNCTIONS::logMsg(3, this->debug_QPIGS(QPIGS_values)); 
      }
      return _funct_return;    
    }

int PV_INVERTER::handle_automation(int _hour, int _min,  bool _CRChardcoded)
    {
      int _funct_return = 0;
      String _result = "";
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
          
          if (_CRChardcoded)
          {
            _funct_return = this->receive(POP01CRC, _result, _CRChardcoded);
          }
          else
          {
            _funct_return = this->receive(POP01, _result, _CRChardcoded);
          }
          if ( _funct_return == 0)                   
          {
            // 0 = info msg
            SUPPORT_FUNCTIONS::logMsg(0, "PV_INVERTER: POP01: Output Priority set to Solar/Grid/Battery");
            _POP_status = POP01;
          }
          else
          {
            // Needs to treat errors for better error messages
            // 2 = error msg
            SUPPORT_FUNCTIONS::logMsg(2, "PV_INVERTER: POP01: Failed to set Output Priority to Solar/Grid/Battery. PV_INVERTER Returned: " + _result);
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
          if (_CRChardcoded)
          {
            _funct_return = this->receive(POP02CRC, _result, _CRChardcoded);
          }
          else
          {
            _funct_return = this->receive(POP02, _result, _CRChardcoded);
          }          
          
          if ( _funct_return == 0)                   
          {
            // 0 = info msg
            SUPPORT_FUNCTIONS::logMsg(0, "PV_INVERTER: POP02: Output Priority set to Solar/Battery/Grid");     
            _POP_status = POP02;
          }
          else
          {
            // Needs to treat errors for better error messages
            // 2 = error msg
            SUPPORT_FUNCTIONS::logMsg(2, "PV_INVERTER: POP02: Failed to set Output Priority to Solar/Battery/Grid. PV_INVERTER Returned: " + _result);     
          }
        }
      }
      return _funct_return;
    }


// ******************************************  CRC Functions  ******************************************

uint16_t PV_INVERTER::crc_xmodem_update (uint16_t crc, uint8_t data)
{
  int i;
  crc = crc ^ ((uint16_t)data << 8);
  for (i=0; i<8; i++) {
  if (crc & 0x8000)
    { crc = (crc << 1) ^ 0x1021; } //(polynomial = 0x1021) 
  else
    { crc <<= 1; } 
   }
return crc;
}


uint16_t PV_INVERTER::calc_crc(char *msg, int n)
{
  uint16_t _CRC = 0;
  switch ( this->getProtocol() )     // select protocol for the right CRC calculation.
  {
    case 0:  //no crc needed
    break;

    case 1:   // protocol 1 CRC HPS (PowMr ) MTTP inverter
    case 2:    // protocol 2 CRC for MAX MPPT
    default:
      
      while( n-- )
        {
        _CRC = this->crc_xmodem_update( _CRC, (uint16_t)*msg++);
        this->ESPyield();
        }
    break;

    
    case 3:   // for future
    break;

  }
return( _CRC );
}

String PV_INVERTER::addCRC(String _cmd)
{
  String _CRC="";
  if ( !_cmd.isEmpty() )
      {
      uint16_t _vgCrcCheck;
      int _vInvCommandLen = 0;
      char _s[6];
      int _CRC1; 
      int _CRC2;
     
      _vInvCommandLen = _cmd.length();
      char _vInvCommandArray[_vInvCommandLen]; //Arrary define

      _cmd.toCharArray(_vInvCommandArray, _vInvCommandLen + 1);
  
      //Calculating CRC
      _vgCrcCheck = this->calc_crc((_vInvCommandArray),_vInvCommandLen);

      // CRC returns two characters - these need to be separated and send as HEX to PV_INVERTER
      String _vgCrcCheckString = String(_vgCrcCheck, HEX);
      String _vCrcCorrect = _vgCrcCheckString.substring(0,2) + " " + _vgCrcCheckString.substring(2,4);
        
      //CRC are returned as B7A9 - need to separate them and change from ASCII to HEX
      _vCrcCorrect.toCharArray(_s, 6);
      sscanf(_s, "%x %x", &_CRC1, &_CRC2);  
  
      _CRC = char(_CRC1);   // add CRC byte 1
      _CRC += char(_CRC2);   // add CRC byte 2
      //this->debugMsg("CRC: " + string2hex(_CRC));    
    }
return _CRC;
}

int PV_INVERTER::getProtocol()
  {
    return _inverter_protocol;
  }

void PV_INVERTER::setProtocol(int _protocol_no)
  {
    _inverter_protocol = _protocol_no;
  }
