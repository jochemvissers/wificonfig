#ifndef wificonfig_h
#define wificonfig_h
#include <ESPAsyncWebServer.h>
//#include <memory>

// system configuration
#define WIFIAP_PREFIX "wificonfig-%04d"
#define WIFIAP_PASS "password"
#define WIFI_USER "admin"
#define WIFI_PASS "wifi"

//internal defines
#define CONFIG_START 32
#define CONFIG_VERSION "aaa"

// poor mans strlcpy_P. works because:
// If the resulting string would be longer than n-1 characters, the remaining characters are discarded and not stored, but counted for the value returned by the function.
// A terminating null character is automatically appended after the content written.
#define strlcpy_P(d,s,n) snprintf_P((d),(n),"%s",(s))

class wificonfig {
  public:
    wificonfig(); // Constructor
    void init();
    void init(AsyncWebServer * server);
    void startwifi();
    void savehandle(AsyncWebServerRequest * request);
    void resethandle(AsyncWebServerRequest * request);
    void showhtml(AsyncWebServerRequest * request);
    void sendconfigjson(AsyncWebServerRequest * request);
    void sendconfigtxt(AsyncWebServerRequest * request);
    void sendConnectionStateAndValues(AsyncWebServerRequest * request);
    void statushtml(AsyncWebServerRequest * request);
  private:
    struct strConfig {
    char version[4] = ""; //Needs to be first!
    char ssid[16] = "";
    char password[16] = "";
    char devicename[16] = "";
    char mqttaddress[16] = "";
    char mqttpassword[16] = "";
    byte  ip[4] = {0,0,0,0};
    byte  netmask[4] = {0,0,0,0};
    byte  gateway[4] = {0,0,0,0};
    boolean dhcp = true;
    boolean softap = false;
    }   config;

    bool resetConfig();
    bool loadConfig();
    void saveConfig();

};

#endif
