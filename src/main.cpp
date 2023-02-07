#include <Arduino.h>
#include <esp_dmx.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "ArduinoJson.h"
#include "AsyncJson.h"
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include "credentials.h"

int transmitPin = 27;
int receivePin = 4;
int enablePin = 5;

//Slider variables
bool useSLiderColors = false;
int slider_r = 0;
int slider_g = 0;
int slider_b = 0;

int sliderArrayLength = 0;

int slider_index = 0;
int slider_color_index = 0;

int slider_speed = 1;

dmx_port_t dmxPort = 1;

byte data[5];

class sliderColorData
{
public:
  int red;
  int green;
  int blue;
};

sliderColorData sliderColorDataArray[44];

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
  useSLiderColors = false;
  request->send(200); 
});

void initializeSliderColors(int startFrom){
  slider_r = map(0, 0, 100, sliderColorDataArray[startFrom].red, sliderColorDataArray[startFrom+1].red);
  slider_g = map(0, 0, 100, sliderColorDataArray[startFrom].green, sliderColorDataArray[startFrom+1].green);
  slider_b = map(0, 0, 100, sliderColorDataArray[startFrom].blue, sliderColorDataArray[startFrom+1].blue);
  slider_index = 0;
}

AsyncCallbackJsonWebHandler *handler2 = new AsyncCallbackJsonWebHandler("/set-slider-colors", [](AsyncWebServerRequest *request, JsonVariant &json)
{
  Serial.println("Got JSON");
  JsonArray obj = json.as<JsonArray>();
  sliderArrayLength = obj.size();
  for (int i = 0; i < sliderArrayLength; i++)
  {
    sliderColorDataArray[i].red = obj[i]["red"];
    sliderColorDataArray[i].green = obj[i]["green"];
    sliderColorDataArray[i].blue = obj[i]["blue"];
    Serial.println(sliderColorDataArray[i].red);
    Serial.println(sliderColorDataArray[i].green);
    Serial.println(sliderColorDataArray[i].blue);
  }
  useSLiderColors = true;
  initializeSliderColors(0);
  request->send(200);
});

AsyncCallbackJsonWebHandler *handler3 = new AsyncCallbackJsonWebHandler("/set-slider-speed", [](AsyncWebServerRequest *request, JsonVariant &json)
{
  Serial.println("Got JSON");
  JsonObject obj = json.as<JsonObject>();
  int rawSpeed = obj["speed"];
  //Serial.println(rawSpeed);
  slider_speed = floor(100*pow(256-rawSpeed, 0.33));
  //Serial.println(slider_speed);
  request->send(200);
});

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
  server.addHandler(handler);
  server.addHandler(handler2);
  server.addHandler(handler3);
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

  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
}

void updateSliderColors(){
  if(useSLiderColors){
    if(slider_index == slider_speed){
      if(slider_color_index == sliderArrayLength-2){
        slider_color_index = 0;  
      }
      else{
        slider_color_index++;  
      }
      slider_index = 0;
    }
    data[2] = slider_r;
    data[3] = slider_g;
    data[4] = slider_b;

    slider_index++;
    slider_r = map(slider_index, 0, slider_speed, sliderColorDataArray[slider_color_index].red, sliderColorDataArray[slider_color_index+1].red);
    slider_g = map(slider_index, 0, slider_speed, sliderColorDataArray[slider_color_index].green, sliderColorDataArray[slider_color_index+1].green);
    slider_b = map(slider_index, 0, slider_speed, sliderColorDataArray[slider_color_index].blue, sliderColorDataArray[slider_color_index+1].blue);

    //Serial.println(slider_r);
  }
}

void loop()
{
  updateSliderColors();
  dmx_write_packet(dmxPort, data, 5);
  dmx_send_packet(dmxPort, 5);

  dmx_wait_send_done(dmxPort, DMX_PACKET_TIMEOUT_TICK);

  delay(10);
  // put your main code here, to run repeatedly:
}