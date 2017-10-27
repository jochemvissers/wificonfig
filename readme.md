# Introduction

Wificonfig is a library that provides a basic wifi network configuration page to ESP8266 Arduino projects that work with [AsyncWebServer](link)

When the device starts connection to the configured access point is made.
In parallel wificonfig sets up an access point for wifi configuration.
This access point disappears after 5 minutes.  

Configuration is saved to EEPROM.

Code is based on:
[tzapu/wifimanager](...)
[ESP_WebConfig_V1.1.3](...)
[Tasmota](...)
