#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define CLIENT_TIMEOUT 5000
#define TOKEN TOKEN_FROM_COMPILER

IPAddress ip(192, 168, 1, 253);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const char* ssid     = SSID_FROM_COMPILER;
const char* password = PASSWD_FROM_COMPILER;

WiFiServer server(29, 3);
WiFiClient client;

unsigned long timerExecution;

int convertAndSend(String &jsonReq);

void connectClient(){
    WiFi.config(ip, gateway, subnet);
    WiFi.setAutoConnect(true);
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    WiFi.setHostname("ALARM_TL_ESP32");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup(){
  Serial.begin(115200);
  connectClient();
  //Set slave number
  server.begin();
  Serial.println("Partito...!");
  Wire.begin();
}//setup

void loop() {
  if (client = server.available()) {
      Serial.println("_CONN");
      String currentLine = "";
      unsigned long timeout = millis();
      while (client.connected()) {
        if (millis() - timeout > CLIENT_TIMEOUT){
          client.stop();
          Serial.println("_DISCONN");
          break;
        }
        if (client.available()) {
          char c = client.read();
          if (c == '$') {
            currentLine.trim();
            if (currentLine != ""){
              int action = convertAndSend(currentLine);
              if (action == 2){
                  client.write("{ \"status\" : \"OK\" }\n");
              }
              currentLine = "";
              client.stop();
              Serial.println("_DISCONN");
              break;
            }
          } else {
            currentLine += c;
          }

        }//while
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
        connectClient();
    }
}//loop

int convertAndSend(String& jsonReq){
  char json[jsonReq.length()+1];
  jsonReq.toCharArray(json, jsonReq.length()+1);
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()){
    Serial.println("Json non valido.");
  } else {
      if (!root.containsKey("token") && !root.containsKey("action"))
        return -1;
      char buffer[128];
      strcpy(buffer, root["token"]);
      String token = String(buffer);
      if (token != TOKEN)
        return -1;
      int action = root["action"];
      Serial.println(jsonReq);
      Serial.println("Action : "+String(action));
      return action;
  }
  return -1;
}
