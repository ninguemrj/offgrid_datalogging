#include "webserver_inverter.h"

////////// Date and time function for error mesages //////////
extern String _errorDateTime();


void WEBSERVER_INVERTER::begin(String _ssid, String _password, PV_INVERTER *_inv, PV_INVERTER::pipVals_t (*_SQL_daily_QPIGS)[SQL_ARRAY_SIZE+1])
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


  //--- PV 40 readings of acActivePower from SQL and parse as Json  -----------------

  _server.on("/sqlDaily.json", HTTP_GET, [_SQL_daily_QPIGS](AsyncWebServerRequest *request)
  {
    uint32_t teste = millis();
    StaticJsonDocument<19000> doc;
    JsonArray arr1 = doc.createNestedArray();
    for (int i=0; i<SQL_ARRAY_SIZE; i++)
    {
      arr1[0] = (uint64_t)(*_SQL_daily_QPIGS)[i]._unixtime*1000;
      arr1[1] = (*_SQL_daily_QPIGS)[i].acActivePower;
      doc.add(arr1);
      
      #if defined (ESP8266) || (defined ESP32)
        yield();
      #endif 
    } 
    Serial.println("time json: " + String(millis()-teste));
    String response;
    serializeJson(doc, response);
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
