#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

void setup()
{
  Serial.begin(115200);
  WiFiManager wifiManager;

  // reset saved settings
  // wifiManager.resetSettings();

  wifiManager.autoConnect("GCtl-Client-AutoConnect");

  // if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
}

void loop()
{
  // put your main code here, to run repeatedly:
}