#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <Arduino.h>
#include <ESP8266WiFi.h>      //ESP8266 WiFi driver
#include <PubSubClient.h>     //MQTT Library
#include <queue>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include "webPage.h"

WiFiClient wclient;//WiFi Object
PubSubClient client(wclient);//MQTT Object
ESP8266WebServer server(80);
/* PIN DEFINES */
#define R_PIN 12
#define G_PIN 13
#define B_PIN 14

//const char *ssid =  "mmadn-tplink-2.4";   // cannot be longer than 32 characters!
//const char *pass =  "e63a3cb942";   //wifi password
char mqttBroker[40] =  "";
char mqttUsername[40] = "";
char mqttPassword[40] = "";
char mqttPort[6] = "";

char mdnsName[40] = "";

char mqttID [10];
char mqttCommandChannel [50];
char mqttStatusChannel [50];
char mqttDebugChannel [50];

bool debugBool = true;

std::queue <String> mqttCommandQueue;

int colour_temp = 1500;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

void debug(String debugMessage){
  if(debugBool){
    Serial.println(debugMessage);
  }
}

void commandDecode(String rawCommand){
  DynamicJsonBuffer jsonBuffer(100); //JSON buffer
  JsonObject& root = jsonBuffer.parseObject(rawCommand);//extract JSON data
  if (!root.success())
  {
    debug("JSON Error:");
    debug(rawCommand);
    return;
  }
  Serial.println(rawCommand);
  String command = root["command"];
  int r = root["r"];
  int g = root["g"];
  int b = root["b"];
  int timespan = root["t"];
  int kelvin = root["k"];
  float brightness = root["b"];

  //compare to a known request
  if(command == ""){
      debug("No command set!");
  } else if(command == "set_rgb")  {
     //straight set an RGB colour
     debug("Command: set_rgb");
     set_rgb(r,g,b);
  } else if(command =="fade_rgb") {
     //straight set an RGB colour
     debug("Command: fade_rgb");
     fade_rgb(r,g,b,timespan);
  } else  if(command == "fade_kelvin"){
   //fade to a new colour temperature
     debug("Command: fade_kelvin");
     fade_kelvin(kelvin, timespan);
  } else if(command =="set_kelvin"){
   //fade to a new colour temperature
     debug("Command: set_kelvin");
     set_colour_temperature(kelvin);
  } else if(command == "set_brightness"){
   //fade to a new colour temperature
     debug("Command: set_brightness");
     set_brightness(brightness);
  } else if(command == "off"){
     //turn lights off
    debug("Command: lights_off");
    rgb_off(); //make sure the lights start off!
  } else {
    debug("That command is  not supported");
  }
}

//Callback for MQTT data receive
void mqttRecvCallback(char* topic, byte* payload, unsigned int length) {
  char command[length];
  memcpy(command,payload,length);
  mqttCommandQueue.push(command);
}

void mqttSetup(){
  client.setServer(mqttBroker, atoi(mqttPort));
  client.setCallback(mqttRecvCallback);
  sprintf(mqttID,"%d",ESP.getChipId());
  sprintf(mdnsName,"RGB-%d",ESP.getChipId());
  sprintf(mqttStatusChannel,"esp/rgblight/%d/status", ESP.getChipId());
  sprintf(mqttCommandChannel,"esp/rgblight/%d/command", ESP.getChipId());
  sprintf(mqttDebugChannel,"esp/rgblight/%d/debug", ESP.getChipId());
  debug("MQTT Topics:");
  debug(mqttStatusChannel);
  debug(mqttCommandChannel);
  debug(mqttDebugChannel);
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup () {
  // put your setup code here, to run once:
   Serial.begin(115200);
   Serial.println();

   //WiFiManager
   WiFiManager wifiManager;
   //clean FS and settings, for testing
   //SPIFFS.format();
   //wifiManager.resetSettings();

   //read configuration from FS json
   openConfig();

   WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqttBroker, 40);
   WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqttPort, 5);
   WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqttUsername, 40);
   WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqttPassword, 40);



   //set config save notify callback
   wifiManager.setSaveConfigCallback(saveConfigCallback);

   //add all your parameters here
   wifiManager.addParameter(&custom_mqtt_server);
   wifiManager.addParameter(&custom_mqtt_port);
   wifiManager.addParameter(&custom_mqtt_username);
   wifiManager.addParameter(&custom_mqtt_password);


   //sets timeout until configuration portal gets turned off
   //useful to make it all retry or go to sleep
   //in seconds
   //wifiManager.setTimeout(120);

   //fetches ssid and pass and tries to connect
   //if it does not connect it starts an access point with the specified name
   //here  "AutoConnectAP"
   //and goes into a blocking loop awaiting configuration
   if (!wifiManager.autoConnect("RGB_WiFi", "password")) {
     Serial.println("failed to connect and hit timeout");
     delay(3000);
     //reset and try again, or maybe put it to deep sleep
     ESP.reset();
     delay(5000);
   }

   //if you get here you have connected to the WiFi
   Serial.println("WiFi connected");


   //read updated parameters
   strcpy(mqttBroker, custom_mqtt_server.getValue());
   strcpy(mqttPort, custom_mqtt_port.getValue());
   strcpy(mqttUsername, custom_mqtt_username.getValue());
   strcpy(mqttPassword, custom_mqtt_password.getValue());


   //save the custom parameters to FS
   if (shouldSaveConfig) {
     saveConfig();
   }

   Serial.println("local ip");
   Serial.println(WiFi.localIP());
   mqttSetup();
   setup_rgb(R_PIN,G_PIN,B_PIN,1); //setup RGB LED strip
   fade_rgb(0,255,0,500);
   fade_rgb(0,0,0,500); //show were up and running

   //server stuff
   if (MDNS.begin(mdnsName)) {
     Serial.println("MDNS responder started");
   }

   server.on("/", HTTP_POST, handleSubmit);
   server.on("/", HTTP_GET, handleRoot);
   server.onNotFound(handleNotFound);


  server.begin();
  Serial.println("HTTP server started");
}

void reconnect(void) {
  while (!client.connected()) { // Loop until we're reconnected
     debug("MQTT Connecting...");
    // Attempt to connect, set LWT so that an offline status will be set when powered down
    if (client.connect(mqttID, mqttUsername, mqttPassword,mqttStatusChannel,1,1,"offline")) {
      debug("MQTT Connected");// Connected
      client.subscribe(mqttCommandChannel); //subscribe to command topic
      client.publish(mqttStatusChannel, "online",1); //let them know we're online

    } else {
      debug("MQTT Failed");
      delay(500);// Wait before retrying
    }
  }
}


void loop(){
  client.loop(); //update MQTT client
  server.handleClient();
  if (WiFi.status() == WL_CONNECTED)
  {
     if (!client.connected())
     {
       reconnect();
     }
   }
  while(!mqttCommandQueue.empty()){
    commandDecode(mqttCommandQueue.front());
    mqttCommandQueue.pop();
  }
}

void openConfig(){
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(mqttBroker, json["mqtt_server"]);
          strcpy(mqttPort, json["mqtt_port"]);
          strcpy(mqttUsername, json["mqtt_username"]);
          strcpy(mqttPassword, json["mqtt_password"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}

void saveConfig(){
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["mqtt_server"] = mqttBroker;
  json["mqtt_port"] = mqttPort;
  json["mqtt_username"] = mqttUsername;
  json["mqtt_password"] = mqttPassword;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

void handleRoot() {
  debug("Handling Root");
  server.send(200,"text/html",RGB_SELECTOR_PAGE);
}

void handleSubmit() {
  debug("Handle Submit");
  if(server.hasArg("command")) {
    mqttCommandQueue.push(server.arg("command"));
  } else {
    debug("no command argument");
  }
  server.send(200, "text/plain", "");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
