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

#include <Int64String.h>
#include <WiFi.h>
#include "HTTPClient.h"
#include <EasyDDNS.h>

#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
//#include <ArduinoJson.h>
#include "SQLITE_inverter.h"
#include "config.h"

class WEBSERVER_INVERTER
{
  public:
    AsyncWebServer _server = AsyncWebServer(80);

    WEBSERVER_INVERTER() {};

    void begin(const char* _ssid, const char* _password, PV_INVERTER *_inv, SQLITE_INVERTER *_SQL_INV);
    void runLoop();


  private:

};

#endif
