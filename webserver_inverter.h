#ifndef WEBSERVER_INVERTER_H
#define WEBSERVER_INVERTER_H

#include <Arduino.h>

#include "PVinverter.h"
#include "String.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>


class WEBSERVER_INVERTER
{
  public:
    AsyncWebServer _server = AsyncWebServer(80);

    WEBSERVER_INVERTER() {};

    void begin(String _ssid, String _password, PV_INVERTER::pipVals_t *_thisPIP);

  private:

};

#endif