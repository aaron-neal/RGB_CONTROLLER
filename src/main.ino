#include <Arduino.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>      //ESP8266 WiFi driver
#include <PubSubClient.h>     //MQTT Library
#include <queue>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include "webPage.h"
#include "rgbTools.h"


WiFiClient wclient;//WiFi Object
PubSubClient client(wclient);//MQTT Object
ESP8266WebServer server(80);

/* PIN DEFINES */
#define R_PIN 12
#define G_PIN 13
#define B_PIN 14

#define mqttRetries 3

/* MQTT configuration*/
std::queue <String> mqttCommandQueue;
char mqttBroker[40] =  "";
char mqttUsername[40] = "";
char mqttPassword[40] = "";
char mqttPort[6] = "";
char mqttID [10];
char mqttCommandChannel [50];
char mqttStatusChannel [50];
char mqttDebugChannel [50];
char bootCommand [100];

int mqttFailCount = 0;

bool shouldSaveConfig = false; //flag for saving data

char mdnsName[40] = "";
char deviceName[40] = "";

bool debugBool = true;
int boot = 0; //first boot check

void setup () {
   Serial.begin(115200);
   debug("WiFi RGB Booting");
   setupRGB(R_PIN,G_PIN,B_PIN); //setup RGB LED strip

   WiFiManager wifiManager;
   //clean FS and settings, for testing
   //SPIFFS.format();
   //wifiManager.resetSettings();

   openConfig();

   wifiManager.setDebugOutput(false);
   WiFiManagerParameter custom_device_name("name", "device name", deviceName, 40);
   WiFiManagerParameter custom_mqtt_server("server", "mqtt server (optional)", mqttBroker, 40);
   WiFiManagerParameter custom_mqtt_port("port", "mqtt port (optional)", mqttPort, 5);
   WiFiManagerParameter custom_mqtt_username("username", "mqtt username (optional)", mqttUsername, 40);
   WiFiManagerParameter custom_mqtt_password("password", "mqtt password (optional)", mqttPassword, 40);
   wifiManager.addParameter(&custom_device_name);
   wifiManager.addParameter(&custom_mqtt_server);
   wifiManager.addParameter(&custom_mqtt_port);
   wifiManager.addParameter(&custom_mqtt_username);
   wifiManager.addParameter(&custom_mqtt_password);

   //set AP callbacks
   wifiManager.setSaveConfigCallback(saveConfigCallback);
   wifiManager.setAPCallback(configModeCallback);

   if (!wifiManager.autoConnect("RGB_WiFi", "password")) {
     debug("failed to connect and hit timeout");
     delay(3000);
     //reset and try again, or maybe put it to deep sleep
     ESP.reset();
     delay(5000);
   }

   //if you get here you have connected to the WiFi
   debug("WiFi connected");
   fadeRGBBlocking({0,0,0},100);
   //read updated parameters
   strcpy(deviceName, custom_device_name.getValue());
   strcpy(mqttBroker, custom_mqtt_server.getValue());
   strcpy(mqttPort, custom_mqtt_port.getValue());
   strcpy(mqttUsername, custom_mqtt_username.getValue());
   strcpy(mqttPassword, custom_mqtt_password.getValue());

   //save the custom parameters to FS
   if (shouldSaveConfig) {
     saveConfig();
   }

   debug("IP Address:");
   debug(WiFi.localIP().toString());
   mqttSetup(); //sets all the mqtt topics based on the chip-id

   //server stuff
   if (MDNS.begin(mdnsName)) {
     debug("MDNS responder started");
     MDNS.addService("http", "tcp", 80);
   }

   server.on("/", HTTP_POST, handleSubmit);
   server.on("/", HTTP_GET, handleRoot);
   server.onNotFound(handleNotFound);

   server.begin();
   debug("HTTP server started");
}

void loop(){
  if(boot == 0){
    //perform a green glow to indicate we are connected to the network
    fadeRGBBlocking({0,255,0},500); //glow green
    fadeRGBBlocking({0,0,0},500); //show were up and running
    delay(500);
    if(strlen(bootCommand) != 0){
      mqttCommandQueue.push(bootCommand);
    }
    boot=1;
  }

  client.loop(); //update MQTT client
  server.handleClient(); //update server handling
  rgbLoop();
  if (WiFi.status() == WL_CONNECTED)
  {
     if (!client.connected() && strlen(mqttBroker) != 0 && mqttFailCount < mqttRetries) //dont use MQTT if no broker set
     {
       reconnect();
     }
   }
  while(!mqttCommandQueue.empty()){
    commandDecode(mqttCommandQueue.front());
    mqttCommandQueue.pop();
  }
}

/*-----------------Command Decoding-----------------*/
void commandDecode(String rawCommand){
  debug(rawCommand);
  DynamicJsonBuffer jsonBuffer(100); //JSON buffer
  JsonObject& root = jsonBuffer.parseObject(rawCommand);//extract JSON data

  if (!root.success())
  {
    debug("JSON Error");
    return;
  }

  String command = root["command"];
  rgb colourData;
  colourData.r = root["r"];
  colourData.g = root["g"];
  colourData.b = root["b"];
  int timespan = root["timespan"];
  int kelvin = root["kelvin"];
  double brightness = root["brightness"];

  //compare to a known request
  if(command == ""){
      debug("No command set!");
  } else if(command == "setRGB")  {
     //straight set an RGB colour
     debug("Command: setRGB");
     setRGB(colourData);
  } else if(command =="fadeRGB") {
     //straight set an RGB colour
     debug("Command: fadeRGB");
     fadeRGB(colourData,timespan);
  } else  if(command == "fadeKelvin"){
   //fade to a new colour temperature
     debug("Command: fade_kelvin");
     fadeKelvin(kelvin, timespan);
  } else if(command =="setKelvin"){
   //fade to a new colour temperature
     debug("Command: set_kelvin");
     setKelvin(kelvin);
  } else if(command == "setBrightness"){
   //fade to a new colour temperature
     debug("Command: setBrightness");
     setBrightness(brightness);
  } else if(command == "off"){
     //turn lights off
    debug("Command: Off");
    off();
  } else if(command == "resetSettings") {
    debug("Command: resetSettings");
    factoryReset();
  } else if(command == "setBootColour") {

    debug("Command: setBootColour");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["command"] = "fadeRGB";
    json["r"] = colourData.r;
    json["g"] = colourData.g;
    json["b"] = colourData.b;
    json["timespan"] = 500;
    json.printTo(Serial);
    json.printTo(bootCommand, sizeof(bootCommand));
    saveConfig();
  } else if(command == "clearBootColour") {
     //turn lights off
    debug("Command: clearBootColour");
    String x = "";
    x.toCharArray(bootCommand,sizeof(bootCommand));
    saveConfig();
  } else if(command == "restart") {
    ESP.reset();
    delay(5000);
  } else if(command == "startRandomColours") {
    startRandom(timespan);
  } else if(command == "stopRandomColours") {
    stopRandom();
  } else {
    debug("That command is not supported!");
  }
}

/*-----------------WifiManager Callbacks-----------------*/
//callback notifying us of the need to save config
void saveConfigCallback () {
  debug("Should save config");
  shouldSaveConfig = true;
}
//calback notifying of config mode
void configModeCallback (WiFiManager *myWiFiManager) {
  fadeRGBBlocking({100,0,0},500);
}
/*-----------------MQTT Functions-----------------*/
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

void reconnect(void) {
  int i = 0;
  while (i<2 && !client.connected()) { // attempt 3 connections
     debug("MQTT Connecting...");
    // Attempt to connect, set LWT so that an offline status will be set when powered down
    if (client.connect(mqttID, mqttUsername, mqttPassword,mqttStatusChannel,1,1,"offline")) {
      debug("MQTT Connected");// Connected
      client.subscribe(mqttCommandChannel); //subscribe to command topic
      client.publish(mqttStatusChannel, "online",1); //let them know we're online
    } else {
      debug("MQTT Failed");
      mqttFailCount++;
      delay(100);// Wait before retrying
    }
    i++;
  }
}

/*-----------------Configuration load / save-----------------*/
void openConfig(){
  debug("mounting FS...");
  if (SPIFFS.begin()) {
    debug("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      debug("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        debug("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          debug("\nparsed json");
          bool jsonComplete = true;
          if (json.containsKey("device_name"))
          {
              strcpy(deviceName, json["device_name"]);
          } else { jsonComplete = false;}
          if (json.containsKey("mqtt_server"))
          {
              strcpy(mqttBroker, json["mqtt_server"]);
          } else { jsonComplete = false;}
          if (json.containsKey("mqtt_port"))
          {
              strcpy(mqttPort, json["mqtt_port"]);
          } else { jsonComplete = false;}
          if (json.containsKey("mqtt_username"))
          {
              strcpy(mqttUsername, json["mqtt_username"]);
          } else { jsonComplete = false;}
          if (json.containsKey("mqtt_password"))
          {
            strcpy(mqttPassword, json["mqtt_password"]);
          } else { jsonComplete = false;}
          if (json.containsKey("boot_command"))
          {
              strcpy(bootCommand, json["boot_command"]);
          } else { jsonComplete = false;}

          if(!jsonComplete){
            //the json is not complete, so factory reset
            factoryReset();
          }

        } else {
          debug("failed to load json config");
          factoryReset();
        }
      }
    }
  } else {
    debug("failed to mount FS");
  }
}

void saveConfig(){
  debug("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["device_name"] = deviceName;
  json["mqtt_server"] = mqttBroker;
  json["mqtt_port"] = mqttPort;
  json["mqtt_username"] = mqttUsername;
  json["mqtt_password"] = mqttPassword;
  json["boot_command"] = bootCommand;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    debug("failed to open config file for writing");
    factoryReset();
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

void factoryReset(){
  fadeRGBBlocking({100,0,0},100);
  WiFiManager wifiManager;
  SPIFFS.format();
  wifiManager.resetSettings();
  delay(3000);
  fadeRGBBlocking({0,0,0},100);
  ESP.reset();
  delay(5000);
}

/*-----------------Web server Handlers-----------------*/
void handleRoot() {
  debug("Handling Root");
  String webPage = RGB_SELECTOR_PAGE;
  webPage.replace("{{}}",deviceName);
  server.send(200,"text/html",webPage);
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

/*-----------------Debug Handler-----------------*/
void debug(String debugMessage){
  if(debugBool){
    Serial.println(debugMessage);
  }
}
