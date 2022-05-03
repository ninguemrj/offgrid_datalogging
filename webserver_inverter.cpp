#include "webserver_inverter.h"

////////// Date and time function for error mesages //////////
extern String _errorDateTime();


void WEBSERVER_INVERTER::begin(String _ssid, String _password, PV_INVERTER::pipVals_t *_thisPIP, PV_INVERTER::pipVals_t (*_SQL_QPIGS)[40])
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

  //--- PV CHARGER POWER from pipvals  -----------------

  _server.on("/PVPower", HTTP_GET, [_thisPIP](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_thisPIP->PV1_chargPower).c_str() );
  });

  _server.on("/PVPower.json", HTTP_GET, [_SQL_QPIGS](AsyncWebServerRequest *request)
  {
    StaticJsonDocument<1536> doc;
    JsonArray unixtime = doc.createNestedArray("unixtime");
    JsonArray PVPower = doc.createNestedArray("PVPower");
    for (int i=0; i<40; i++)
    {
      unixtime.add((*_SQL_QPIGS)[i]._unixtime);
      PVPower.add((*_SQL_QPIGS)[i].acActivePower); 
      
      #if defined (ESP8266) || (defined ESP32)
      yield();
      #endif 
    } 
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response );
  });

  
  //--- ACTIVE POWER from pipvals ---------------------
  _server.on("/ActivePower", HTTP_GET, [_thisPIP](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_thisPIP->acActivePower).c_str());
  });

  //--- BATTERY VOLTAGE from pipvals ---------------------
  _server.on("/BatVoltage", HTTP_GET, [_thisPIP](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_thisPIP->batteryVoltage/100.00).c_str());
  });

  //--- BATTERY CHARGE from pipvals ---------------------
  _server.on("/BatCharge", HTTP_GET, [_thisPIP](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_thisPIP->batteryChargeCurrent).c_str());
  });

  //--- BATTERY DISCHARGE from pipvals ---------------------
  _server.on("/BatDischarge", HTTP_GET, [_thisPIP](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", String(_thisPIP->batteryDischargeCurrent).c_str());
  });

  //--- Start web server ------------
  _server.begin();

}
