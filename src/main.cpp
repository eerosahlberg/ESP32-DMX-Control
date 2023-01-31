#include <Arduino.h>
#include <esp_dmx.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "ArduinoJson.h"
#include "AsyncJson.h"
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include "credentials.h"

int transmitPin = 2;
int receivePin = 4;
int enablePin = 5;

dmx_port_t dmxPort = 1;

byte data[5];

int packetCounter = 44;
byte incrementValue = 0;

AsyncWebServer server(80);

AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/set-color", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                       {
  Serial.println("Got JSON");
  JsonObject obj = json.as<JsonObject>();
  data[2] = obj["red"];
  data[3] = obj["green"];
  data[4] = obj["blue"];
  request->send(200); });

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD); // credentials are set in file credentials.h
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("esp32"))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  else
  {
    Serial.println("mDNS responder started");
  }

  MDNS.addService("http", "tcp", 80);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", String(), false); });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style.css", "text/css"); });

  server.on("/red", HTTP_GET, [](AsyncWebServerRequest *request)
            {
             
              data[0] = 0;
              data[1] = 255;
              data[2] = 255;
              data[3] = 0;
              data[4] = 0;
              request->send(SPIFFS, "/index.html", String(), false); });
  server.on("/green", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              data[0] = 0;
              data[1] = 255;
              data[2] = 0;
              data[3] = 255;
              data[4] = 0;
              request->send(SPIFFS, "/index.html", String(), false); });
  server.on("/blue", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              data[0] = 0;
              data[1] = 255;
              data[2] = 0;
              data[3] = 0;
              data[4] = 255;
              request->send(SPIFFS, "/index.html", String(), false); });
  /*
server.on("/set-color", HTTP_GET, [](AsyncWebServerRequest *request)
{

    data[0] = 0;
    data[1] = 255;
    data[2] = request->getParam("red")->value().toInt();
    data[3] = request->getParam("green")->value().toInt();
    data[4] = request->getParam("blue")->value().toInt();
    request->send(200, "text/plain", "OK"); });
*/
  server.addHandler(handler);
  server.begin();
}

void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  dmx_config_t dmxConfig = DMX_DEFAULT_CONFIG;
  dmx_param_config(dmxPort, &dmxConfig);
  dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);

  int QueueSize = 0;
  int interrupPriority = 1;
  dmx_driver_install(dmxPort, DMX_MAX_PACKET_SIZE, QueueSize, NULL, interrupPriority);

  dmx_set_mode(dmxPort, DMX_MODE_WRITE);
  data[0] = 0;
  data[1] = 255;
  data[2] = 0;
  data[3] = 255;
  data[4] = 0;

  initWiFi();
}

void rainbow()
{
  data[0] = 0;
  data[1] = 255;
  data[2] = packetCounter;
  data[3] = 255 - packetCounter;
  data[4] = 0;
  packetCounter += incrementValue;
  if (packetCounter > 255)
  {
    packetCounter = 255;
    incrementValue = -1;
  }
  if (packetCounter < 0)
  {
    packetCounter = 0;
    incrementValue = 1;
  }
}

void loop()
{
  dmx_write_packet(dmxPort, data, 5);
  dmx_send_packet(dmxPort, 5);

  dmx_wait_send_done(dmxPort, DMX_PACKET_TIMEOUT_TICK);

  delay(10);
  // put your main code here, to run repeatedly:
}