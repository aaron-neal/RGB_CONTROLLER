#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <queue>



AsyncMqttClient mqttClient;

const char *ssid =  "mmadn-tplink-2.4";   // cannot be longer than 32 characters!
const char *pass =  "e63a3cb942";   //wifi password
const char *mqttBroker =  "tfs.aaronneal.co.uk";
const char *mqttUsername = "smartFactory";
const char *mqttPassword = "e11slboro";

char mqttCommandChannel [50];
char mqttStatusChannel [50];
char mqttDebugChannel [50];

bool debugBool = false;


std::queue <String> mqttCommandQueue;

int colour_temp = 1500;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

#define R_PIN 12
#define G_PIN 13
#define B_PIN 14

void debug(const char* debugMessage){
  if(debugBool){
    if(mqttClient.connected()){
      mqttClient.publish(mqttDebugChannel, 2, false, debugMessage);
    }
    Serial.println(debugMessage);
  }
}

void commandDecode(String command){
  if(command.startsWith("set_rgb"))
  {
     //straight set an RGB colour
     debug("Command: set_rgb");
     command.replace("set_rgb{",""); //remove headers and tail characters
     command.replace("}","");
     int r,g,b;
     r = getValue(command, ',', 0).toInt();
     g = getValue(command, ',', 1).toInt();
     b = getValue(command, ',', 2).toInt();
     set_rgb(r,g,b);
  }
  if(command.startsWith("fade_rgb"))
  {
     //straight set an RGB colour
     debug("Command: fade_rgb");
     command.replace("fade_rgb{",""); //remove headers and tail characters
     command.replace("}","");
     int r,g,b, timespan;
     r = getValue(command, ',', 0).toInt();
     g = getValue(command, ',', 1).toInt();
     b = getValue(command, ',', 2).toInt();
     timespan = getValue(command, ',', 3).toInt();
     fade_rgb(r,g,b,timespan);
  }
  if(command.startsWith("fade_kelvin")){
   //fade to a new colour temperature
     debug("Command: fade_kelvin");
     command.replace("fade_kelvin{",""); //remove headers and tail characters
     command.replace("}","");
     int kelvin,timespan;
     kelvin = getValue(command, ',', 0).toInt();
     timespan = getValue(command, ',', 1).toInt();
     fade_kelvin(kelvin, timespan);
  }
  if(command.startsWith("set_kelvin")){
   //fade to a new colour temperature
      debug("Command: set_kelvin");
     command.replace("set_kelvin{",""); //remove headers and tail characters
     int kelvin;
     kelvin = getValue(command, '}', 0).toInt();
     set_colour_temperature(kelvin);
  }

  if(command.startsWith("set_brightness")){
   //fade to a new colour temperature
     debug("Command: set_brightness");
     command.replace("set_brightness{",""); //remove headers and tail characters
     float brightness;
     brightness = getValue(command, '}', 0).toFloat();
     set_brightness(brightness);
  }

  if(command.startsWith("off")){
   //turn lights off
  debug("Command: lights_off");
  rgb_off(); //make sure the lights start off!
  }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String temp = payload;
  mqttCommandQueue.push(temp);
}



void onMqttConnect() {
  Serial.printf("** Connected to %s **\n",mqttBroker);
  mqttClient.subscribe(mqttCommandChannel, 2);
  mqttClient.publish(mqttStatusChannel, 2, true, "online");
  Serial.printf("Subscribing to %s\n",mqttCommandChannel);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.printf("** Disconnected from %s **\n",mqttBroker);
  Serial.printf("Reconnecting to %s ...\n",mqttBroker);
  mqttClient.connect();
}

void mqttSetup(){
  char mqttID [10];
  sprintf(mqttID,"%d",ESP.getChipId());
  sprintf(mqttStatusChannel,"esp/office/rgblight/%d/status", ESP.getChipId());
  sprintf(mqttCommandChannel,"esp/office/rgblight/%d/command", ESP.getChipId());
  sprintf(mqttDebugChannel,"esp/office/rgblight/%d/debug", ESP.getChipId());

  Serial.println("Setting up MQTT");
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(mqttBroker, 1883);
  mqttClient.setKeepAlive(5).setWill(mqttStatusChannel, 2, true, "offline").setCredentials(mqttUsername, mqttPassword).setClientId(mqttID);
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}


void setup () {
 delay(2000);
 Serial.begin(115200);
 Serial.println("");

 WiFi.begin(ssid, pass);
 Serial.print("WiFi Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  mqttSetup();
  setup_rgb(R_PIN,G_PIN,B_PIN,1); //setup RGB LED strip


}


void loop(){

while(!mqttCommandQueue.empty()){
  commandDecode(mqttCommandQueue.front());
  mqttCommandQueue.pop();
}

}

//Helper function to return value from a string, uses a common separator such as ',' or '{'
String getValue(String data, char separator, int index)
{
    int maxIndex = data.length()-1;
    int j=0;
    String chunkVal = "";

    for(int i=0; i<=maxIndex && j<=index; i++)
    {
      chunkVal.concat(data[i]);

      if(data[i]==separator)
      {
        j++;

        if(j>index)
        {
          chunkVal.trim();
          return chunkVal;
        }

        chunkVal = "";
      }
    }
}
