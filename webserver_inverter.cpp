#include "webserver_inverter.h"

////////// Date and time function for error mesages //////////
extern String _errorDateTime();


void WEBSERVER_INVERTER::begin(String _ssid, String _password, PV_INVERTER *_inv, SQLITE_INVERTER *_SQL_INV)
{
      //--- Initialize ESP SPIFFS (flash filesystem) to recover the index.html file --------
      if(!SPIFFS.begin()){
        Serial.println(_errorDateTime() + "--- ERROR: WERBERSER_INVERTER: An Error has occurred while mounting SPIFFS");
        return;
      }
      //--- Connect to Wi-Fi ---------------------
      WiFi.begin(_ssid.c_str(), _password.c_str());
      DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
      DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
      Serial.print("Connecting to WiFi.");
      while (WiFi.status() != WL_CONNECTED) 
      {
        delay(100);
        Serial.print(".");
      }
      Serial.println(" CONECTED !");
    
      //--- Print ESP32 Local IP Address  ------------
      Serial.println(WiFi.localIP());

      //--- DDNS Service  ------------
      EasyDDNS.service("freemyip");
      EasyDDNS.client("casamagalhaes.freemyip.com", "827f49a93329949405531079"); // Enter your DDNS Domain & Token
    
      // Get Notified when your IP changes
      EasyDDNS.onUpdate([&](const char* oldIP, const char* newIP){
        SUPPORT_FUNCTIONS::logMsg(0, "WEBSERVER_INVERTER::begin(): EasyDDNS - IP Change Detected: " + String(newIP));        
      });      

  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html");
  });

  _server.on("/plug-on.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index2.html");
  });

    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/chart.html");
  });
  
  //--- PV CHARGER POWER from pipvals  -----------------

  _server.on("/PVPower", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_inv->QPIGS_values.PV1_chargPower).c_str() );
  });


  //--- PV CHARGER STATUS from pipvals  -----------------

  _server.on("/charger_status", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
    String _response = "";

    if (_inv->QPIGS_values.DevStat_Chargingstatus == 1)
    {
      if (_inv->QPIGS_values.ChargerSourcePriority == 2)
      {
        _response = "sun_plug";
      }
      else
      {
        if (_inv->QPIGS_values.ChargerSourcePriority == 3)
        {
          _response = "sun";
        }
        if (_inv->QPIGS_values.ChargerSourcePriority == 0)
        {
          _response = "plug";
        }
      }
    }
    else
    {
      _response = "off";
    }
    
    request->send(200, "text/plain", _response );
  });


  //--- PV BATTERY VOLTS from pipvals  -----------------

  _server.on("/battery_volts", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
    String _response = String((float)_inv->QPIGS_values.batteryVoltage/100.00);
    request->send(200, "text/plain", _response );
  });


  //--- PV 288 readings of acActivePower from SQL and parse as Json  -----------------

  _server.on("/sqlDaily.json", HTTP_GET, [_SQL_INV](AsyncWebServerRequest *request)
  {
    uint32_t teste = millis();
    String _daily_date = "";
    String response = "";
    
    if (request->hasParam("daily_date")) {
    // New date informed as argument ==> Defines new daily date to fetch data from SQL DB (background)

        _daily_date = request->getParam("daily_date")->value();
        SUPPORT_FUNCTIONS::logMsg(0, "WEBSERVER_INVERTER::begin(): /sqlDaily.json: HTTP-GET Paramenter: " + _daily_date);          // 0 = INFO msg

        // Defines new daily date to fetch data from SQL DB (background)
        _SQL_INV->set_dailyDate(strtoul(_daily_date.c_str(), NULL, 0));

        response = "[[\"Updating...\"]]"; //keeping JSON format
    } else {

    // NO ARGUMENTS ==> Return current DAILY_QPIGS DATA

        yield();
        if (_SQL_INV->daily_data_updated)
        {
          SUPPORT_FUNCTIONS::logMsg(0, "WEBSERVER_INVERTER::begin(): /sqlDaily.json: WITHOUT PARAMETERS: Daily data ready: preparing JSON");
    //--- BEGIN: Prepare JSON string -------------------------------
          String response_time          = "[";
          String response_PV            = "[";
          String response_ACPower       = "[";
          String response_BatV          = "[";
          String response_Bat_charg     = "[";
          String response_Bat_discharg  = "[";
          
          bool _first = true;
          
          for (int i=0; i<SQL_ARRAY_SIZE; i++) 
          {
              if (_first) 
              {
                _first = false;
              }
              else
              { 
                response_time         += ",";
                response_PV           += ",";
                response_ACPower      += ",";
                response_BatV         += ",";
                response_Bat_charg    += ",";
                response_Bat_discharg += ",";
              }
              response_time         += int64String((uint64_t)(_SQL_INV->SQL_daily_QPIGS[i]._unixtime - __FUSO__)*1000);
              response_PV           += String((_SQL_INV->SQL_daily_QPIGS[i].PV1_chargPower));
              response_ACPower      += String((_SQL_INV->SQL_daily_QPIGS[i].acActivePower));
              response_BatV         += String((_SQL_INV->SQL_daily_QPIGS[i].batteryVoltage/100.00));
              response_Bat_charg    += String((_SQL_INV->SQL_daily_QPIGS[i].batteryChargeCurrent));
              response_Bat_discharg += String((_SQL_INV->SQL_daily_QPIGS[i].batteryDischargeCurrent));
          }

          response_time += "],";
          response_PV += "],";
          response_ACPower += "],";
          response_BatV += "],";
          response_Bat_charg += "],";
          response_Bat_discharg += "]";

          response = "[";
          response += response_time;
          response += response_PV;
          response += response_ACPower;
          response += response_BatV;
          response += response_Bat_charg;
          response += response_Bat_discharg;
          response += "]";    
    //--- END: Prepare JSON string -------------------------------
            Serial.println("time json: " + String(millis()-teste));
        }
        else
        {
          SUPPORT_FUNCTIONS::logMsg(0, "WEBSERVER_INVERTER::begin(): /sqlDaily.json: WITHOUT PARAMETERS: Daily data Updating yet...");
          response = "[[\"Updating...\"]]";
          delay(100);
        }
    }
    
    request->send(200, "application/json", response );
  });


  //--- QPIRI information as Json  -----------------

  _server.on("/QPIRI.json", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
        SUPPORT_FUNCTIONS::logMsg(0, "WEBSERVER_INVERTER::begin(): /QPIRI.json: preparing JSON");

    //--- BEGIN: Prepare JSON string -------------------------------
    String response = "";
    response = "[";
    response += String(_inv->QPIRI_values.BatteryUnderVoltage/10.0);
    response += ",";
    response += String(_inv->QPIRI_values.BatteryBulkVoltage/10.0);
    response += ",";
    response += String(_inv->QPIRI_values.BatteryFloatVoltage/10.0);
    response += ",";
    response += String(_inv->QPIRI_values.BatteryReChargeVoltage/10.0);
    response += "]";
    //--- END: Prepare JSON string -------------------------------
    
    request->send(200, "application/json", response );
  });


  
  //--- ACTIVE POWER from pipvals ---------------------
  _server.on("/ActivePower", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_inv->QPIGS_values.acActivePower).c_str());
  });

  //--- BATTERY VOLTAGE from pipvals ---------------------
  _server.on("/BatVoltage", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_inv->QPIGS_values.batteryVoltage/100.00).c_str());
  });

  //--- BATTERY CHARGE from pipvals ---------------------
  _server.on("/BatCharge", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_inv->QPIGS_values.batteryChargeCurrent).c_str());
  });

  //--- BATTERY DISCHARGE from pipvals ---------------------
  _server.on("/BatDischarge", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_inv->QPIGS_values.batteryDischargeCurrent).c_str());
  });


/// ICONS   ////////////////////////////////////////////////////////////////

  _server.on("/sun-on.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/sun-on.png");
  });

  _server.on("/sun-off.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/sun-off.png");
  });

  _server.on("/battery-on.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/battery-on.png");
  });

  _server.on("/battery-off.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/battery-off.png");
  });

  _server.on("/electric-pole.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/electric-pole.png");
  });

  _server.on("/plug-off.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/plug-off.png");
  });

  _server.on("/plug-on.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/plug-on.png");
  });


  //--- DEBUG INFO from pipvals ---------------------
  _server.on("/debug", HTTP_GET, [_inv](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", _inv->debug_QPIGS(_inv->QPIGS_values));
  });


  //--- Start web server ------------
  _server.begin();

}


void WEBSERVER_INVERTER::runLoop()
{
  EasyDDNS.update(10000);
}
