#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>


ESP8266WebServer server(80);

bool output_state[8] = {false, false, false, false, false, false, false, false};
bool output_target[8] = {false, false, false, false, false, false, false, false};
int humidity_sensor_values[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
long flowsens_value = -1;
bool should_reset_flowsens_value = false;

unsigned long millis_now;

// INTERVAL: set output state to target
unsigned long prev_millis_output;
unsigned long const intv_millis_output = 200;

// INTERVAL: get sensor measurements
unsigned long prev_millis_sensors;
unsigned long const intv_millis_sensors = 60000;

// INTERVAL: dead man's switch for outputs
// if the server does not contact the client again in this period, the outputs will turn off
unsigned long prev_millis_dms;
unsigned long const intv_millis_dms = 11000;


// This function returns 'false' if any of the 8 current output channel states mismatches with the output target states
bool output_matching_target() {
  for (int i=0; i<8; i++) {
    if (output_state[i] != output_target[i]) return false;
  }
  return true;
}

// set the physical outputs to the state they should be in.
void setOutput() {
  for (int i = 0; i<8; i++) {
    // TODO:
    // set physical OUTPUTs!
    output_state[i] = output_target[i];
  }
}

// set target to all off, and update instantly!
void instant_all_off() {
  for (int i = 0; i<8; i++) {
    output_target[i] = false;
  }
}

void server_handle_notFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void server_handle_get_status() {
  const int capacity = JSON_OBJECT_SIZE(13);
    StaticJsonDocument<capacity> doc;
    doc["relay1"] = output_state[0] ? 1 : 0;
    doc["relay2"] = output_state[1] ? 1 : 0;
    doc["relay3"] = output_state[2] ? 1 : 0;
    doc["relay4"] = output_state[3] ? 1 : 0;
    doc["relay5"] = output_state[4] ? 1 : 0;
    doc["relay6"] = output_state[5] ? 1 : 0;
    doc["relay7"] = output_state[6] ? 1 : 0;
    doc["relay8"] = output_state[7] ? 1 : 0;
    doc["matchesTarget"] = output_matching_target() ? 1 : 0;
    doc["changedByDMS"] = 0;
    doc["rssi"] = WiFi.RSSI();
    char jsonout[1024];
    serializeJson(doc, jsonout);
    server.send(200, "application/json", jsonout);
}

void server_handle_post_setOutput() {
  
}

void server_set_routing() {
  server.on("/status", HTTP_GET, server_handle_get_status);
  server.on("/setOutput", HTTP_POST, server_handle_post_setOutput);
}


void setup()
{
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  WiFiManager wifiManager;

  // reset saved WiFi settings
  // wifiManager.resetSettings();

  wifiManager.autoConnect("GCtl-Client-AutoConnect");

  // WiFi connection OK
  Serial.println("connected...yeey :)");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Start up HTTP server
  server.onNotFound(server_handle_notFound);
  server_set_routing();
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  millis_now = millis();
  server.handleClient();

  // check for output's DMS
  if (millis_now - prev_millis_dms >= intv_millis_dms) {
    prev_millis_dms = millis_now;

    // if we got here, it means the server didnt send a request and all updates should be disabled!
    // prev_millis_dms is set to millis_now when receiving a command from the server, so we should not get here normally
    instant_all_off();
  }

  if (millis_now - prev_millis_output >= intv_millis_output) {
    prev_millis_output = millis_now;

    // Only do anything if the state of the outputs is not as it should be
    if (!output_matching_target()) setOutput();
  }

  
}