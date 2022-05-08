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

#ifndef SUPPORT_FUNCTIONS_H
  #include "support_functions.h"
#endif

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "SQLITE_inverter.h"
#include "config.h"

class WEBSERVER_INVERTER
{
  public:
    AsyncWebServer _server = AsyncWebServer(80);

    WEBSERVER_INVERTER() {};

    void begin(String _ssid, String _password, PV_INVERTER *_inv, PV_INVERTER::pipVals_t (*_SQL_daily_QPIGS)[288]);


  private:

};

#endif
