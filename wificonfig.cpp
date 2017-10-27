#include <Arduino.h>
#include <wificonfig.h>
#include <wificonfigpages.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <Ticker.h>

#define DEBUG_ESP_PORT Serial

#ifdef DEBUG_ESP_PORT
  #define DEBUG_MSG(...) DEBUG_ESP_PORT.println( __VA_ARGS__ )
  #define DEBUG_MSGP(...) DEBUG_ESP_PORT.printlnP( __VA_ARGS__ )
#else
  #define DEBUG_MSG(...)
  #define DEBUG_MSGP(...)
#endif

Ticker killAPTimer;

void disableAP(void){
  DEBUG_MSG(F("WF: Disabling AP\r\n"));
  WiFi.enableAP(false);
};

void wificonfig::startwifi(void){
  // start wifi mode
  if (config.softap==false){
    DEBUG_MSG(F("WF: Configuring Wifi in client mode"));
    WiFi.begin(config.ssid, config.password);
    if (!config.dhcp)
    {
      DEBUG_MSG(F("WF: STATIC IP. To be implemented!"));
    //  WiFi.config(staticIP, gateway, subnet);
      WiFi.config(IPAddress(config.ip[0],config.ip[1],config.ip[2],config.ip[3] ),  IPAddress(config.gateway[0],config.gateway[1],config.gateway[2],config.gateway[3] ) , IPAddress(config.netmask[0],config.netmask[1],config.netmask[2],config.netmask[3] ));
    }
    WiFi.enableSTA(true);       // set mode to WiFi station
    WiFi.setAutoConnect(true);   // automatic connect
    WiFi.setAutoReconnect(true); // automatic reconnect
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      DEBUG_MSG(".");
    }
    DEBUG_MSG(F("WF: Connected"));

    // setup soft AP
    IPAddress local_IP(192,168,4,1);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);

    char APname[20];
    char passw[20];
    //strncpy_P(passw, WIFIAP_PREFIX, sizeof(passw)-1); // copy password; dont overwrite null character
    strlcpy_P(passw, WIFIAP_PASS, sizeof(passw)); // copy password
    snprintf_P(APname, sizeof(APname), PSTR(WIFIAP_PREFIX), ESP.getChipId() & 0x1FFF);
    Serial.printf("WF: APname=%s,APpass=%s\r\n",APname, passw);
    WiFi.enableAP(true);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(APname, passw);
    killAPTimer.once(300,   disableAP); // set timer to disable AP-mode
  }else{
    // setup permanent soft AP
    DEBUG_MSG(F("WF: Configuring Wifi in AP-mode"));
    IPAddress local_IP(config.ip[0],config.ip[1],config.ip[2],config.ip[3]);
    IPAddress gateway(config.gateway[0],config.gateway[1],config.gateway[2],config.gateway[3]);
    IPAddress subnet(config.netmask[0],config.netmask[1],config.netmask[2],config.netmask[3]);
    WiFi.enableAP(true);
    WiFi.enableSTA(false);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(config.ssid, config.password);
  }
};

wificonfig::wificonfig(void){};
void wificonfig::init(void){
  DEBUG_MSG(F("WF: init() not implemented\n"));
};

void wificonfig::init(AsyncWebServer * server){
  // load configuration
  if(loadConfig()){
      DEBUG_MSG(F("WF: load config OK!\n"));
  }else{
      DEBUG_MSG(F("WF: load config FAILED!\n"));
      resetConfig();
      ESP.reset();
  }

  // configure webserver
  server->on("/wifi", HTTP_GET, std::bind(&wificonfig::showhtml, this, std::placeholders::_1));
  server->on("/status", HTTP_GET, std::bind(&wificonfig::statushtml, this, std::placeholders::_1));
  server->on("/wificonfig/save", HTTP_POST, std::bind(&wificonfig::savehandle, this, std::placeholders::_1));
  server->on("/wificonfig/reset", HTTP_ANY, std::bind(&wificonfig::resethandle, this, std::placeholders::_1));
  server->on("/wificonfig/values", HTTP_GET, std::bind(&wificonfig::sendconfigtxt, this, std::placeholders::_1));
  server->on("/wificonfig/connectionstate", HTTP_ANY, std::bind(&wificonfig::sendConnectionStateAndValues, this, std::placeholders::_1));
  server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send(200, "text/css", FPSTR(HTML_WIFICONF_STYLE));
  });
  DEBUG_MSG(F("WF: init\n"));
};

void wificonfig::showhtml(AsyncWebServerRequest * request){
  #ifdef WIFI_USER
    #ifdef WIFI_PASS
    if(!request->authenticate(WIFI_USER, WIFI_PASS))
      return request->requestAuthentication();
    #endif
  #endif

  DEBUG_MSG(F("WF: show html\n"));
  String page = FPSTR(HTTP_HEAD_);
  page.replace("{v}", "Wifi Config");
  page += FPSTR(HTML_WIFICONF_SCRIPT);
  //page += FPSTR(HTTP_STYLE);
  //page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END_);
  page += FPSTR(HTML_WIFICONF_MAIN);
  page += FPSTR(HTML_WIFICONF_FORMBEGIN);
  page += FPSTR(HTML_WIFICONF_FORM);
  page += FPSTR(HTML_WIFICONF_FORMEND);
  page += FPSTR(HTML_WIFICONF_NETWORKSTATUS);
  page += FPSTR(HTTP_END_);
  request->send(200, "text/html", page);
};

void wificonfig::savehandle(AsyncWebServerRequest * request){
  #ifdef WIFI_USER
    #ifdef WIFI_PASS
    if(!request->authenticate(WIFI_USER, WIFI_PASS))
      return request->requestAuthentication();
    #endif
  #endif
  DEBUG_MSG(F("WF: save handle\n"));

  //List all parameters
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){ //p->isPost() is also true
      Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }

  //checkbox values are false when absent
  config.dhcp = false;
  config.softap = false;

  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if (p->name() == "devicename") {p->value().toCharArray(config.devicename, sizeof(config.devicename));   }
    if (p->name() == "ssid")       {p->value().toCharArray(config.ssid, sizeof(config.ssid));        }
    if (p->name() == "pwd")        {p->value().toCharArray(config.password, sizeof(config.password)); }
    if (p->name() == "dhcp")        { config.dhcp = true; }
    if (p->name() == "softap")      { config.softap = true; }

    if (p->name() == "ip0") { config.ip[0] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "ip1") { config.ip[1] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "ip2") { config.ip[2] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "ip3") { config.ip[3] =  (uint8_t) p->value().toInt(); }

    if (p->name() == "gw0") { config.gateway[0] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "gw1") { config.gateway[1] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "gw2") { config.gateway[2] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "gw3") { config.gateway[3] =  (uint8_t) p->value().toInt(); }

    if (p->name() == "nm0") { config.netmask[0] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "nm1") { config.netmask[1] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "nm2") { config.netmask[2] =  (uint8_t) p->value().toInt(); }
    if (p->name() == "nm3") { config.netmask[3] =  (uint8_t) p->value().toInt(); }

    if (p->name() == "mqaddr")  { p->value().toCharArray(config.mqttaddress, sizeof(config.mqttaddress)); }
    if (p->name() == "mqpass")  { p->value().toCharArray(config.mqttpassword, sizeof(config.mqttpassword)); }
  }
  request->send(200, "text/html", FPSTR(HTML_WIFICONF_SAVED));
  delay(1);
  // save structure to eeprom
  saveConfig();
  DEBUG_MSG(F("WF: saved and rebooting\n"));
  ESP.reset();
};

void wificonfig::sendconfigtxt(AsyncWebServerRequest * request){
  #ifdef WIFI_USER
    #ifdef WIFI_PASS
    if(!request->authenticate(WIFI_USER, WIFI_PASS))
      return request->requestAuthentication();
    #endif
  #endif
	String values ="";
	values += "ssid|" + (String) config.ssid + "|input\n";
	values += "pwd|" +  (String) config.password + "|input\n";
  values += "devicename|" +  (String) config.devicename + "|input\n";
	values += "ip0|" +  (String) config.ip[0] + "|input\n";
	values += "ip1|" +  (String) config.ip[1] + "|input\n";
	values += "ip2|" +  (String) config.ip[2] + "|input\n";
	values += "ip3|" +  (String) config.ip[3] + "|input\n";
	values += "nm0|" +  (String) config.netmask[0] + "|input\n";
	values += "nm1|" +  (String) config.netmask[1] + "|input\n";
	values += "nm2|" +  (String) config.netmask[2] + "|input\n";
	values += "nm3|" +  (String) config.netmask[3] + "|input\n";
	values += "gw0|" +  (String) config.gateway[0] + "|input\n";
	values += "gw1|" +  (String) config.gateway[1] + "|input\n";
	values += "gw2|" +  (String) config.gateway[2] + "|input\n";
	values += "gw3|" +  (String) config.gateway[3] + "|input\n";
	values += "dhcp|" +  (String) (config.dhcp ? "checked" : "") + "|chk\n";
  values += "softap|" +(String) (config.softap ? "checked" : "") + "|chk\n";
  values += "mqaddr|" +(String) config.mqttaddress + "|input\n";
  values += "mqpass|" +(String) config.mqttpassword + "|input\n";
  request->send(200, "text/plain", values);
	Serial.println(__FUNCTION__);
  DEBUG_MSG(values);
};

void wificonfig::resethandle(AsyncWebServerRequest * request){
  #ifdef WIFI_USER
    #ifdef WIFI_PASS
    if(!request->authenticate(WIFI_USER, WIFI_PASS))
      return request->requestAuthentication();
    #endif
  #endif
  DEBUG_MSG(F("WF: reset handle\n"));
  request->send(200, "text/html", FPSTR(HTML_WIFICONF_SAVED));
  resetConfig();
  ESP.reset();
}

void wificonfig::sendConnectionStateAndValues(AsyncWebServerRequest * request){
  #ifdef WIFI_USER
    #ifdef WIFI_PASS
    if(!request->authenticate(WIFI_USER, WIFI_PASS))
      return request->requestAuthentication();
    #endif
  #endif
  DEBUG_MSG(F("WF: send_state_values\n"));
  String state = "N/A";
  String Networks = "";
  if (WiFi.status() == 0) state = F("IDLE");
  else if (WiFi.status() == 1) state = F("NO SSID AVAILABLE");
  else if (WiFi.status() == 2) state = F("SCAN COMPLETED");
  else if (WiFi.status() == 3) state = F("CONNECTED");
  else if (WiFi.status() == 4) state = F("CONNECT FAILED");
  else if (WiFi.status() == 5) state = F("CONNECTION LOST");
  else if (WiFi.status() == 6) state = F("DISCONNECTED");
  WiFi.scanNetworks(true);
  int n = WiFi.scanComplete();
  if (n == -2){
    Networks = F("<font color='#FF0000'>Scan has not been started</font>");
  }else if (n == -1){
    Networks = F("<font color='#FF0000'>Scan results not available</font>");
  }else if (n == 0){
    Networks = F("<font color='#FF0000'>No networks found!</font>");
  }else{
    Networks = "Found " +String(n) + " Networks<br>";
    Networks += F("<table border='0' cellspacing='0' cellpadding='3'>");
    Networks += F("<tr bgcolor='#DDDDDD' ><td><strong>Name</strong></td><td><strong>Quality</strong></td><td><strong>Enc</strong></td><tr>");
    for (int i = 0; i < n; ++i){
    	int quality=0;
    	if(WiFi.RSSI(i) <= -100){
    			quality = 0;
    	}else if(WiFi.RSSI(i) >= -50){
    			quality = 100;
    	}else{
    		quality = 2 * (WiFi.RSSI(i) + 100);
    	}
    	Networks += "<tr><td><a href='javascript:selssid(\""  +  String(WiFi.SSID(i))  + "\")'>"  +  String(WiFi.SSID(i))  + "</a></td><td>" +  String(quality) + "%</td><td>" +  String((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*")  + "</td></tr>";
    }
  Networks += "</table>";
  }
	String values ="";
	values += "connectionstate|" +  state + "|div\n";
	values += "networks|" +  Networks + "|div\n";
	request->send( 200, "text/plain", values);
  //WiFi.scanDelete(); // should clean up this memory
}

void wificonfig::statushtml(AsyncWebServerRequest * request){
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");
  //response->print(request->client()->remoteIP());
  response->print("<h3>General Status</h3>");
  response->print("<ul>");
  response->printf("<li>Build Date & Time</li>");
  response->printf("<li>Uptime</li>");
  response->print("</ul>");

  response->print("<h3>System Status</h3>");
  response->print("<ul>");
  response->printf("<li>Chip ID: %u</li>", ESP.getFlashChipId());
  response->printf("<li>Chip Real Size: (kB)%u</li>", ESP.getFlashChipRealSize() / 1024);
  response->printf("<li>Chip Flash Size: (kB)%u</li>", ESP.getSketchSize() / 1024);
  response->printf("<li>Free program space: (kB)%u</li>", ESP.getFreeSketchSpace() / 1024);
  response->printf("<li>Last reset reason: (kB)%s</li>", ESP.getResetReason().c_str());
  response->printf("<li>Chip speed: MHz</li>");
  response->printf("<li>Chip mode: </li>");
  response->printf("<li>Boot version: </li>");
  response->printf("<li>Core version: </li>");
  response->printf("<li>SDK version: </li>");
  response->printf("<li>Free memory: (kB) </li>");
  response->print("</ul>");

  response->print("<h3>Network Status</h3>");
  response->print("<ul>");
  response->printf("<li>Hostname</li>");
  response->printf("<li>MAC address</li>");
  response->printf("<li>SSID</li>");
  response->printf("<li>RSSI</li>");
  response->printf("<li>Local IP</li>");
  response->printf("<li>Gateway</li>");
  response->printf("<li>Netmask</li>");
  response->print("</ul>");

  // Serial.print("Chip Speed: ");
  // Serial.println(ESP.getFlashChipSpeed());
  // Serial.print("Chip Mode: ");
  // Serial.println(ESP.getFlashChipMode());
  // Serial.print("Boot Version: ");
  // Serial.println(String(ESP.getBootVersion()));
  // Serial.print("Core Version: ");
  // Serial.println(ESP.getCoreVersion());
  // Serial.print("SDK Version: ");
  // Serial.println(String(ESP.getSdkVersion()));
  // Serial.print("Last reset reason: ");
  // Serial.println(String(ESP.getResetReason()));
  //
  // Serial.print("Free Memmory (kB)");
  // Serial.println(String(freeMem / 1024));
  response->print("</ul>");
  request->send(response);

  int freeMem = ESP.getFreeHeap();
  Serial.println();
  Serial.print("Chip ID: ");
  Serial.println(ESP.getFlashChipId());
  Serial.print("Chip Real Size: (kB)");
  Serial.println(String(ESP.getFlashChipRealSize() / 1024));
  Serial.print("Chip Flash Size: (kB)");
  Serial.println(String(ESP.getSketchSize() / 1024));
  Serial.print("Free program space: (kB)");
  Serial.println(String(ESP.getFreeSketchSpace() / 1024));
  Serial.print("Chip Speed: ");
  Serial.println(ESP.getFlashChipSpeed());
  Serial.print("Chip Mode: ");
  Serial.println(ESP.getFlashChipMode());
  Serial.print("Boot Version: ");
  Serial.println(String(ESP.getBootVersion()));
  Serial.print("Core Version: ");
  Serial.println(ESP.getCoreVersion());
  Serial.print("SDK Version: ");
  Serial.println(String(ESP.getSdkVersion()));
  Serial.print("Last reset reason: ");
  Serial.println(String(ESP.getResetReason()));

  Serial.print("Free Memmory (kB)");
  Serial.println(String(freeMem / 1024));
}

bool wificonfig::loadConfig(void) {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  EEPROM.begin(1024);
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2]){
      EEPROM.get(CONFIG_START, config);
      EEPROM.end();
      return true;
  }else{
      DEBUG_MSG(F("WF: Config version mismatch\n"));
      EEPROM.end();
      return false;
  }
}

bool wificonfig::resetConfig(void) {
  // complete reset
  strConfig emptyconfig;
  emptyconfig.version[0] = CONFIG_VERSION[0];
  emptyconfig.version[1] = CONFIG_VERSION[1];
  emptyconfig.version[2] = CONFIG_VERSION[2];

  EEPROM.begin(1024);
  Serial.println("Configuration reset");
  EEPROM.put(CONFIG_START,emptyconfig);
  EEPROM.commit();
  EEPROM.end();
}

void wificonfig::saveConfig(void) {
  config.version[0] = CONFIG_VERSION[0];
  config.version[1] = CONFIG_VERSION[1];
  config.version[2] = CONFIG_VERSION[2];
  EEPROM.begin(1024);
  Serial.println("Configuration saved");
  EEPROM.put(CONFIG_START,config);
  EEPROM.commit();
  EEPROM.end();
}
