/**************************************************************************************
 * SQLITE_INVERTER.cpp (v0.1)
 * Library to be used (or not) with the PV_inverter library.
 * Created to store read data in SD card for backup purposes.
 * 
 * 
 * TODO:
 *    - Change code to enable removing and replacing
 *      SD card without turning off the ESP32;
 *    - Receive information if the data was updated on cloud or not.
 *      If not, store in a separated file the offline readings; 
 *    - When ESP32 become online again, main code will call a function 
 *      to SQLITE_INVERTER provide to WIFI module the off line readings
 *      to update the cloud server;
 *    - Create function to store QPIRI data (different file name);
 *    - Create function to store other Inverter feedback commands;
 * 
 * Contributors: Larryl79, Richardo Jr ( NiguemJr )
 * Version info: 
 *    - v0.1: Initial implementation for testing only
 **************************************************************************************/
 
#include "SQLite_inverter.h"

////////// Date and time function for error mesages //////////
extern String _errorDateTime();


void SQLITE_INVERTER::begin()
{

}


void SQLITE_INVERTER::ask_latest_SQL_QPIGS()
{
/////// SAMPLE CODE FOR SQLite3 SELECT STATEMENT /////////////////
    // Clears previous Select results from RES pointer
    sqlite3_finalize(res);

    //Clears previous QPIGS info stored in SQL_daily_QPIGS
    this->clear_SqlQPIGS();
 
     
     uint32_t teste2 = millis();

    uint32_t _begin_SearchDateTime = (uint32_t)SUPPORT_FUNCTIONS::convertToUnixtime(2022, 05, 06, 0, 0, 0); 
    uint32_t _end_SearchDateTime   = (uint32_t)SUPPORT_FUNCTIONS::convertToUnixtime(2022, 05, 06, 23, 59, 59); 
    
    uint32_t _time_split = 5 * 60 ;         // 15 minutes * 60 seconds in order to average all reads withing each 5 minutes
    uint32_t _count_time_split = 0;         //from 0 to SQL_ARRAY_SIZE (for time split will be from 1 to SQL_ARRAY_SIZE+1)
    uint32_t _count_within_split_reads = 0; // for counting how many rows from SqlDB were read within 5 minutes (for averaging only)

    String _SQL = String("Select * from 'QPIGS' WHERE (") + _begin_SearchDateTime + String(" <= _unixtime AND _unixtime <= ") + _end_SearchDateTime + String(") ORDER BY _unixtime ASC");
    
    // 3 = DEBUG msg
    SUPPORT_FUNCTIONS::logMsg(3, _SQL);
    
    rc = sqlite3_prepare_v2(db1, _SQL.c_str()  , 1000, &res, &tail);
    uint32_t _rows = 0;

    while (sqlite3_step(res) == SQLITE_ROW) 
    {
        yield();
        // Is the current position read time stamp higher than the limit of the current ´_count_time_split´?
        // -> YES = Average previous accumulated readings by dividing with '_count_within_split_reads' AND 1) increment ´_count_time_split´ AND 2) zero '_count_within_split_reads';
        // -> NO = Leave it to continue accumulating readings and counting '_count_within_split_reads'
        
        if (sqlite3_column_int(res, 0) >= (_begin_SearchDateTime + (_time_split * (_count_time_split + 1))))
        {
          // BEING OF Averaging previous accumulated readings by dividing the SUM with "how many rows were accumulated"
          this->SQL_daily_QPIGS[_count_time_split].gridVoltage              = this->SQL_daily_QPIGS[_count_time_split].gridVoltage              / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].gridFrequency            = this->SQL_daily_QPIGS[_count_time_split].gridFrequency            / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].acOutput                 = this->SQL_daily_QPIGS[_count_time_split].acOutput                 / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].acFrequency              = this->SQL_daily_QPIGS[_count_time_split].acFrequency              / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].acApparentPower          = this->SQL_daily_QPIGS[_count_time_split].acApparentPower          / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].acActivePower            = this->SQL_daily_QPIGS[_count_time_split].acActivePower            / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].loadPercent              = this->SQL_daily_QPIGS[_count_time_split].loadPercent              / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].busVoltage               = this->SQL_daily_QPIGS[_count_time_split].busVoltage               / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].batteryVoltage           = this->SQL_daily_QPIGS[_count_time_split].batteryVoltage           / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].batteryChargeCurrent     = this->SQL_daily_QPIGS[_count_time_split].batteryChargeCurrent     / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].batteryCharge            = this->SQL_daily_QPIGS[_count_time_split].batteryCharge            / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].inverterTemperature      = this->SQL_daily_QPIGS[_count_time_split].inverterTemperature      / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].PVCurrent                = this->SQL_daily_QPIGS[_count_time_split].PVCurrent                / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].PVVoltage                = this->SQL_daily_QPIGS[_count_time_split].PVVoltage                / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].PVPower                  = this->SQL_daily_QPIGS[_count_time_split].PVPower                  / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].batterySCC               = this->SQL_daily_QPIGS[_count_time_split].batterySCC               / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].batteryDischargeCurrent  = this->SQL_daily_QPIGS[_count_time_split].batteryDischargeCurrent  / _count_within_split_reads;
          this->SQL_daily_QPIGS[_count_time_split].PV1_chargPower           = this->SQL_daily_QPIGS[_count_time_split].PV1_chargPower           / _count_within_split_reads;

          // Zeros the '_count_within_split_reads' for calculate next averaging
          _count_within_split_reads = 0;

          // Increments '_count_time_split' to move next array item and check the next 300 seconds (5minutes) split
          _count_time_split++;
        
          // ENDO OF Averaging previous accumulated readings by dividing the SUM with "how many rows were accumulated"
        }
         
        this->SQL_daily_QPIGS[_count_time_split]._unixtime                 = sqlite3_column_int(res, 0); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].gridVoltage              += sqlite3_column_int(res, 1);
        this->SQL_daily_QPIGS[_count_time_split].gridFrequency            += sqlite3_column_int(res, 2);
        this->SQL_daily_QPIGS[_count_time_split].acOutput                 += sqlite3_column_int(res, 3);
        this->SQL_daily_QPIGS[_count_time_split].acFrequency              += sqlite3_column_int(res, 4);
        this->SQL_daily_QPIGS[_count_time_split].acApparentPower          += sqlite3_column_int(res, 5);
        this->SQL_daily_QPIGS[_count_time_split].acActivePower            += sqlite3_column_int(res, 6);
        this->SQL_daily_QPIGS[_count_time_split].loadPercent              += sqlite3_column_int(res, 7);
        this->SQL_daily_QPIGS[_count_time_split].busVoltage               += sqlite3_column_int(res, 8);
        this->SQL_daily_QPIGS[_count_time_split].batteryVoltage           += sqlite3_column_int(res, 9);
        this->SQL_daily_QPIGS[_count_time_split].batteryChargeCurrent     += sqlite3_column_int(res, 10);
        this->SQL_daily_QPIGS[_count_time_split].batteryCharge            += sqlite3_column_int(res, 11);
        this->SQL_daily_QPIGS[_count_time_split].inverterTemperature      += sqlite3_column_int(res, 12);
        this->SQL_daily_QPIGS[_count_time_split].PVCurrent                += sqlite3_column_int(res, 13);
        this->SQL_daily_QPIGS[_count_time_split].PVVoltage                += sqlite3_column_int(res, 14);
        this->SQL_daily_QPIGS[_count_time_split].PVPower                  += sqlite3_column_int(res, 15);
        this->SQL_daily_QPIGS[_count_time_split].batterySCC               += sqlite3_column_int(res, 16);
        this->SQL_daily_QPIGS[_count_time_split].batteryDischargeCurrent  += sqlite3_column_int(res, 17);
        this->SQL_daily_QPIGS[_count_time_split].DevStat_SBUpriority       = sqlite3_column_int(res, 18); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_ConfigStatus      = sqlite3_column_int(res, 19); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_FwUpdate          = sqlite3_column_int(res, 20); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_LoadStatus        = sqlite3_column_int(res, 21); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_BattVoltSteady    = sqlite3_column_int(res, 22); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_Chargingstatus    = sqlite3_column_int(res, 23); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_SCCcharge         = sqlite3_column_int(res, 24); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_ACcharge          = sqlite3_column_int(res, 25); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].batOffsetFan              = sqlite3_column_int(res, 26); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].eepromVers                = sqlite3_column_int(res, 27); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].PV1_chargPower           += sqlite3_column_int(res, 28);
        this->SQL_daily_QPIGS[_count_time_split].DevStat_chargingFloatMode = sqlite3_column_int(res, 29); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_SwitchOn          = sqlite3_column_int(res, 30); // getting only the latest 5min reading
        this->SQL_daily_QPIGS[_count_time_split].DevStat_dustProof         = sqlite3_column_int(res, 31); // getting only the latest 5min reading

        //Serial.println("Column: _unixtime  | Data: " + String(this->SQL_daily_QPIGS[_count_time_split]._unixtime) + "|  ROW num: " + String(_rows));

        // Continue accumulating readings and counting '_count_within_split_reads'
        _count_within_split_reads++;

        // Prepare the row counter for the next row
        _rows ++;
    }
    Serial.println("************************************************************************************************************************");
    // 3 = DEBUG msg
    SUPPORT_FUNCTIONS::logMsg(4, "ROW num: " + String(_rows) + "| time SELECT and averaging each 5 minutes: " + String(millis()-teste2));

    // 3 = DEBUG msg
    SUPPORT_FUNCTIONS::logMsg(3, "03-getMinFreeHeap(): " + String(ESP.getMinFreeHeap()) + "| getMaxAllocHeap(): " + String(ESP.getMaxAllocHeap()) + "|  getHeapSize(): " + String(ESP.getHeapSize())  + "|  getFreeHeap(): " + String(ESP.getFreeHeap()));

}


void SQLITE_INVERTER::clear_SqlQPIGS()
{
  // 3 = DEBUG msg
  for (int _rows = 0; _rows < SQL_ARRAY_SIZE; _rows++)
  {
    yield();
    this->SQL_daily_QPIGS[_rows]._unixtime                = 0;
    this->SQL_daily_QPIGS[_rows].gridVoltage              = 0;
    this->SQL_daily_QPIGS[_rows].gridFrequency            = 0;
    this->SQL_daily_QPIGS[_rows].acOutput                 = 0;
    this->SQL_daily_QPIGS[_rows].acFrequency              = 0;
    this->SQL_daily_QPIGS[_rows].acApparentPower          = 0;
    this->SQL_daily_QPIGS[_rows].acActivePower            = 0;
    this->SQL_daily_QPIGS[_rows].loadPercent              = 0;
    this->SQL_daily_QPIGS[_rows].busVoltage               = 0;
    this->SQL_daily_QPIGS[_rows].batteryVoltage           = 0;
    this->SQL_daily_QPIGS[_rows].batteryChargeCurrent     = 0;
    this->SQL_daily_QPIGS[_rows].batteryCharge            = 0;
    this->SQL_daily_QPIGS[_rows].inverterTemperature      = 0;
    this->SQL_daily_QPIGS[_rows].PVCurrent                = 0;
    this->SQL_daily_QPIGS[_rows].PVVoltage                = 0;
    this->SQL_daily_QPIGS[_rows].PVPower                  = 0;
    this->SQL_daily_QPIGS[_rows].batterySCC               = 0;
    this->SQL_daily_QPIGS[_rows].batteryDischargeCurrent  = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_SBUpriority      = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_ConfigStatus     = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_FwUpdate         = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_LoadStatus       = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_BattVoltSteady   = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_Chargingstatus   = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_SCCcharge        = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_ACcharge         = 0;
    this->SQL_daily_QPIGS[_rows].batOffsetFan             = 0;
    this->SQL_daily_QPIGS[_rows].eepromVers               = 0;
    this->SQL_daily_QPIGS[_rows].PV1_chargPower           = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_chargingFloatMode= 0;
    this->SQL_daily_QPIGS[_rows].DevStat_SwitchOn         = 0;
    this->SQL_daily_QPIGS[_rows].DevStat_dustProof        = 0;
  }
}


uint8_t SQLITE_INVERTER::sd_StoreQPIGS(PV_INVERTER::pipVals_t _thisPIP, bool _stored_online)
{
    /// Benchmark
    uint32_t oldtime = millis();

    //----- Prepared QPIGS INSERT SQL Statement ----------------
    String _QPIGS_line = "INSERT INTO 'QPIGS' VALUES (" +
      String(_thisPIP._unixtime)                + "," +
      String(_thisPIP.gridVoltage)              + "," +
      String(_thisPIP.gridFrequency)            + "," +
      String(_thisPIP.acOutput)                 + "," +
      String(_thisPIP.acFrequency)              + "," +
      String(_thisPIP.acApparentPower)          + "," +
      String(_thisPIP.acActivePower)            + "," +
      String(_thisPIP.loadPercent)              + "," +
      String(_thisPIP.busVoltage)               + "," +
      String(_thisPIP.batteryVoltage)           + "," +
      String(_thisPIP.batteryChargeCurrent)     + "," +
      String(_thisPIP.batteryCharge)            + "," +
      String(_thisPIP.inverterTemperature)      + "," +
      String(_thisPIP.PVCurrent)                + "," +
      String(_thisPIP.PVVoltage)                + "," +
      String(_thisPIP.PVPower)                  + "," +
      String(_thisPIP.batterySCC)               + "," +
      String(_thisPIP.batteryDischargeCurrent)  + "," +
      String(_thisPIP.DevStat_SBUpriority)      + "," +
      String(_thisPIP.DevStat_ConfigStatus)     + "," +
      String(_thisPIP.DevStat_FwUpdate)         + "," +
      String(_thisPIP.DevStat_LoadStatus)       + "," +
      String(_thisPIP.DevStat_BattVoltSteady)   + "," +
      String(_thisPIP.DevStat_Chargingstatus)   + "," +
      String(_thisPIP.DevStat_SCCcharge)        + "," +
      String(_thisPIP.DevStat_ACcharge)         + "," +
      String(_thisPIP.batOffsetFan)             + "," +
      String(_thisPIP.eepromVers)               + "," +
      String(_thisPIP.PV1_chargPower)           + "," +
      String(_thisPIP.DevStat_chargingFloatMode)+ "," +
      String(_thisPIP.DevStat_SwitchOn)         + "," +
      String(_thisPIP.DevStat_dustProof)        +
      ");";
      
    if (VERBOSE_MODE == 1) Serial.println(_errorDateTime() +"-- - VERBOSE: SQLITEr: SQL Cmd line: |" + _QPIGS_line + "|END.");
    

    // Run SQL Insert statement 
    rc = db_exec(db1, _QPIGS_line.c_str());
    if (rc != SQLITE_OK) 
    {
       Serial.println(_errorDateTime() +"--- ERROR: SQLITE: INSERT SQL Cmd error code: " + String(rc));
       sqlite3_close(db1);
       
       return 1;
    }
    
    if (VERBOSE_MODE == 2) Serial.println(_errorDateTime() +"--- VERBOSE: SQLITE: information INSERTed in the database row: |" + String((long)sqlite3_last_insert_rowid(db1)) + "|END.");
    if (VERBOSE_MODE == 1) Serial.println(_errorDateTime() +"--- VERBOSE: SQLITE: MEM USED: |" + String((long)sqlite3_memory_used()) + "|END.");
    if (VERBOSE_MODE == 1) Serial.println(_errorDateTime() +"--- VERBOSE: SQLITE: MEM HighWater: |" + String((long)sqlite3_memory_highwater(1)) + "|END.");

    //sqlite3_close(db1);

  /* /////// SAMPLE CODE FOR SQLite3 SELECT STATEMENT /////////////////
    
    // Clears previous Select results from RES pointer
    sqlite3_finalize(res);
    rc = sqlite3_prepare_v2(db1, "Select * from 'QPIGS'"  , 1000, &res, &tail);
    bool first = true;
    uint32_t rows = 0;
    int _count = 0;
    while (sqlite3_step(res) == SQLITE_ROW) 
    {
//        Serial.println("====================================================================================================");
        if (first) 
        {
          _count = sqlite3_column_count(res);
//          Serial.println("COUNT: " + String(_count));
          
          if (_count == 0) 
          {
 //           Serial.printf("Rec Count: %s\n", sqlite3_changes(db1));
              break;
          }
          for (int i = 0; i<_count; i++) 
          {
 //           Serial.printf("Column Name: %s\n", sqlite3_column_name(res, i));
          }
          first = false;
        }
        _count = sqlite3_column_count(res);
        int32_t test[_count];
        for (int i = 0; i<_count; i++) 
        {
          test[i] =sqlite3_column_int(res, i);
//          Serial.println("Column num: " + String(i) + " | Data: " + String(test[i]));
        }
        rows ++;

    }

   Serial.println("Columns: "+String(_count)+" | rows: "+String(rows)+" elapsed time:" + String(millis() - oldtime));
   sqlite3_close(db1);
*/    
     
  
/*  
  //------------   Check if there is SD and available size ----------------------
  if (card_inserted() != 0) 
  {
    Serial.println(_errorDateTime() + "-- ERROR: SQLITE: QPIGS not stored in SD Card! -----");
    return 1;
  }

  // Convert unixtime from smoothed read to readable date to define file name
  ts = *localtime((time_t*)_thisPIP._unixtime);
  
  //------- file name ----------------

  String _file = "/"+String(ts.tm_year+1900)+String(ts.tm_mon+1)+String(ts.tm_mday)+"_QPIGS.csv";

  //----- Prepared QPIGS string for writing ----------------
  String _QPIGS_line = 
      String(_thisPIP._unixtime)                + "," +
      String(_thisPIP.gridVoltage)              + "," +
      String(_thisPIP.gridFrequency)            + "," +
      String(_thisPIP.acOutput)                 + "," +
      String(_thisPIP.acFrequency)              + "," +
      String(_thisPIP.acApparentPower)          + "," +
      String(_thisPIP.acActivePower)            + "," +
      String(_thisPIP.loadPercent)              + "," +
      String(_thisPIP.busVoltage)               + "," +
      String(_thisPIP.batteryVoltage)           + "," +
      String(_thisPIP.batteryChargeCurrent)     + "," +
      String(_thisPIP.batteryCharge)            + "," +
      String(_thisPIP.inverterTemperature)      + "," +
      String(_thisPIP.PVCurrent)                + "," +
      String(_thisPIP.PVVoltage)                + "," +
      String(_thisPIP.PVPower)                  + "," +
      String(_thisPIP.batterySCC)               + "," +
      String(_thisPIP.batteryDischargeCurrent)  + "," +
      _thisPIP.deviceStatus[0]                  + "," +
      _thisPIP.deviceStatus[1]                  + "," +
      _thisPIP.deviceStatus[2]                  + "," +
      _thisPIP.deviceStatus[3]                  + "," +
      _thisPIP.deviceStatus[4]                  + "," +
      _thisPIP.deviceStatus[5]                  + "," +
      _thisPIP.deviceStatus[6]                  + "," +
      _thisPIP.deviceStatus[7]                  + "," +
      String(_thisPIP.batOffsetFan)             + "," +
      String(_thisPIP.eepromVers)               + "," +
      String(_thisPIP.PV1_chargPower)           + "," +
      _thisPIP.deviceStatus2[0]                 + "," +
      _thisPIP.deviceStatus2[1]                 + "," +
      _thisPIP.deviceStatus2[2];

    if (VERBOSE_MODE == 1)
    {
      Serial.println(_errorDateTime() + "-- VERBOSE: SQLITE: File name: " + _file);
      Serial.println(_errorDateTime() + "-- VERBOSE: SQLITE: QPIGS String |" + _QPIGS_line + "|");
    }

  appendFile(SD, _file.c_str(), _QPIGS_line.c_str());

  */
  return 0;

}


/// PRIVATE FUNCTIONS ///////////////////////////////////////////////////


//// START SQLite3 //////////////////////
int SQLITE_INVERTER::callback(void *data, int argc, char **argv, char **azColName){
   int i;
   Serial.println(_errorDateTime() +"-- VERBOSE: SQLITE: SQLite callback: " + String((const char*)data));
/*   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   */
   return 0;
}

int SQLITE_INVERTER::openDb(const char *filename, sqlite3 **db) {
   int _result = sqlite3_open(filename, db);
   if (_result) {
       Serial.println(_errorDateTime() +"-- ERROR: SQLITE: SQL error: " + String(sqlite3_errmsg(*db)));
       return _result;
   } else {
       if (VERBOSE_MODE == 1) Serial.println(_errorDateTime() +"-- VERBOSE: SQLITE: Open database successfully");
   }
   return _result;
}

int SQLITE_INVERTER::db_exec(sqlite3 *db, const char *sql) {
   //Serial.println(sql);
   uint32_t start = millis();
   int _result = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (_result != SQLITE_OK) {
       Serial.println(_errorDateTime() +"-- ERROR: SQLITE: SQL error: " + String(zErrMsg));
       sqlite3_free(zErrMsg);
   } else {
       if (VERBOSE_MODE == 1) Serial.println(_errorDateTime() +"--  VERBOSE: SQLITE: db_exec function ran SQL Statement successfully");
   }
   if (VERBOSE_MODE == 2)
   {
     Serial.print("Time taken:");
     Serial.println(millis()-start);
   }
   return _result;
}

//// END SQLite3 //////////////////////


//// SD Card File manipulation //////////////////////
uint8_t SQLITE_INVERTER::card_inserted()
{
    if(SD.cardType() == CARD_NONE){
        Serial.println(_errorDateTime() + "-- ERROR: SQLITE: No SD card attached.");
        return 1;
    }
    
    // --- only 0.5mb available ERROR
    if((SD.cardSize() - SD.usedBytes()) < (500 * 1024) )
    {
        Serial.println(_errorDateTime() + "--- ERROR: SQLITE: less than 500kb available, STORE function not executed!");
        return 2;
    }

    // --- less then 100mb available warning
    if((SD.cardSize() - SD.usedBytes()) < (100 * 1024 * 1024) )
    {
        Serial.println(_errorDateTime() + "--- WARNING: SQLITE: less than 100mb available, please change SD Card!");
    }    
    
    if (VERBOSE_MODE == 1)
    {
      Serial.print(_errorDateTime() + "--- VERBOSE: SQLITE_INVERTER: SD Card Size: ");
      Serial.print(SD.cardSize()/1024/1024.00);
      Serial.print(" Mb | Available: ");
      Serial.print((SD.cardSize() - SD.usedBytes()) /1024/1024.00);
      Serial.print(" Mb | Used: ");
      Serial.print(SD.usedBytes() /1024/1024.00);
      Serial.println(" Mb ");
    }
    return 0;
}


// Missing better implementation: Read line by line into an feedback array

void SQLITE_INVERTER::readFile(fs::FS &fs, const char * path){
   File file = fs.open(path);
    if(!file){
        Serial.print("--- ERROR: SQLITE_INVERTER: Failed to open the file for reading: ");
        Serial.println(path);
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void SQLITE_INVERTER::writeFile(fs::FS &fs, const char * path, const char * message)
{
    File file = fs.open(path, FILE_WRITE);
    if(!file)
    {
        Serial.print("--- ERROR: SQLITE_INVERTER: Failed to open file for writing: ");
        Serial.println(path);
        return;
    }
    if(!file.print(message))
    {
        Serial.print("--- ERROR: SQLITE_INVERTER: Failed to write data in the file: ");
        Serial.println(path);
    }
    file.close();
}

void SQLITE_INVERTER::appendFile(fs::FS &fs, const char * path, const char * message)
{
    File file = fs.open(path, FILE_APPEND);
    if(!file)
    {
        Serial.print("--- ERROR: SQLITE_INVERTER: Failed to open file for appending: ");
        Serial.println(path);
        return;
    }
    if(!file.println(message))
    {
        Serial.print("--- ERROR: SQLITE_INVERTER: Failed to append data in the file: ");
        Serial.println(path);
    }
    file.close();
}

void SQLITE_INVERTER::deleteFile(fs::FS &fs, const char * path)
{
    if(!fs.remove(path))
    {
        Serial.print("--- ERROR: SQLITE_INVERTER: Failed to delete file: ");
        Serial.println(path);
    }
}

//// END SD Card File manipulation //////////////////////
