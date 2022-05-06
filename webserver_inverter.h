#ifndef WEBSERVER_INVERTER_H
#define WEBSERVER_INVERTER_H

#ifndef ARDUINO_H
  #include <Arduino.h>
#endif 

#ifndef STRING_H
  #include <string.h>
#endif

#ifndef PVINVERTER_H
  #include "PVinverter.h"
#endif

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "SQLITE_inverter.h"

extern String PV_INVERTER::debug_QPIGS(PV_INVERTER::pipVals_t _thisPIP);
    
class WEBSERVER_INVERTER
{
  public:
    AsyncWebServer _server = AsyncWebServer(80);

    WEBSERVER_INVERTER() {};

    void begin(String _ssid, String _password, PV_INVERTER::pipVals_t *_thisPIP, PV_INVERTER::pipVals_t (*_SQL_QPIGS)[40]);


  private:

};

#endif
