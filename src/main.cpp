#include <Arduino.h>

// For WiFi connectivity & WIFIManager
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFiMulti.h>
#include <ESP_WiFiManager.h>
#include <ESP8266WiFiMulti.h>

// For saving the config
#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266SSDP.h>
#include <LittleFS.h>

// For measurements
// #include <OneWire.h>
// #include <DallasTemperature.h>

// DEFINEs
#define USE_ESP_WIFIMANAGER_NTP false
#define WIFICHECK_INTERVAL 1000L
#define FileFS LittleFS
#define FS_Name "LittleFS"
#define USE_DHCP_IP true
#define USE_CONFIGURABLE_DNS false
#define USE_CUSTOM_AP_IP false
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS 2200L
#define WIFI_MULTI_CONNECT_WAITING_MS 500L
#define NUM_WIFI_CREDENTIALS      2
#define SSID_MAX_LEN            32
#define MIN_AP_PASSWORD_SIZE    8
#define PASS_MAX_LEN            64

// HOSTNAME SETTING
// #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())
#define HOSTNAME "GCtl-ESPClient"

// Function Prototypes and other
uint8_t connectMultiWiFi();
WiFi_AP_IPConfig WM_AP_IPconfig;
WiFi_STA_IPConfig WM_STA_IPconfig;
String Router_SSID;
String Router_Pass;
FS *filesystem = &LittleFS;
ESP8266WiFiMulti wifiMulti;
WM_Config WM_config;

// VERSION SETTING
const char VERSION[] = "v0.0a  ( " __DATE__ " " __TIME__ " )";

uint8_t connectMultiWiFi()
{
  uint8_t status;

  LOGERROR(F("ConnectMultiWiFi with :"));

  if ((Router_SSID != "") && (Router_Pass != ""))
  {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass);
    LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ((String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE))
    {
      LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw);
    }
  }

  LOGERROR(F("Connecting MultiWifi..."));

  int i = 0;

  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ((i++ < 20) && (status != WL_CONNECTED))
  {
    status = WiFi.status();

    if (status == WL_CONNECTED)
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if (status == WL_CONNECTED)
  {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP());
  }
  else
  {
    LOGERROR(F("WiFi not connected"));

    ESP.reset();
  }

  return status;
}


void check_WiFi()
{
  if ((WiFi.status() != WL_CONNECTED))
  {
    Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
    connectMultiWiFi();
  }
}

void check_status()
{
  static long checkstatus_timeout = 0;
  static long checkwifi_timeout = 0;
  static long current_millis;

  current_millis = millis();

  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
  {
    check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }
}

void setup()
{
  Serial.begin(115200);

  while (!Serial)
    ;

  delay(200);

  ESP_WiFiManager ESP_wifiManager("GCtl-ESPClient");
  ESP_wifiManager.setMinimumSignalQuality(-1);
  ESP_wifiManager.setConfigPortalChannel(0);

  Router_SSID = ESP_wifiManager.WiFi_SSID();
  Router_Pass = ESP_wifiManager.WiFi_Pass();

  // Remove this line if you do not want to see WiFi password printed
  Serial.println("ESP Self-Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);
}

void loop()
{
  // put your main code here, to run repeatedly:
  check_status();
}