// This is example file from https://github.com/esp8266/Arduino
// It has been modified for the purpose of OnlineHumidifier project - https://github.com/krzychb/OnlineHumidifier


//
// DHT Sensor Setup
//
#include "DHT.h"
//  dht(DHTPIN, DHTTYPE);
DHT dht(D2, DHT22);
float humidity;


//
// RF 433 transmitter configuration
//
// download - https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/Home/
//
#include <RemoteTransmitter.h>
#define RF433_TR_PIN D1   // pin where RF433 Transmitter is connected to
ActionTransmitter actionTransmitter(RF433_TR_PIN);
bool humidifier;


//
// Wi-Fi Setup
//
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "********";     // your network SSID (name)
const char* password = "********";  // your network password

ESP8266WebServer server(80);

// Onboard LED pin
const int led = D4;

void handleRoot() {
  digitalWrite(led, LOW);
  String message = "hello from esp8266!";
  message += "\nHumidity = ";
  message += (String) humidity;
  message += "%";

  server.send(200, "text/plain", message);
  digitalWrite(led, HIGH);
}

void handleNotFound() {
  digitalWrite(led, LOW);
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
  digitalWrite(led, HIGH);
}

void showControlScreen(void){
  String message;
  message += "<html>";
  message += "<head><meta http-equiv='refresh' content='5'/><title>Online Humidifier</title></head>";
  message += "<body>";
  message += "<h3>Humidifier Control</h3>";
  message += "<p>";
  message += "Humidity : " + (String) humidity + "%<br />";
  message += "Humidifier : " + (String) humidifier + "<br />";
  message += "Operate : ";
  message += "<a href=\"/humidifier/1\">On</a>";
  message += " / ";
  message += "<a href=\"/humidifier/0\">Off</a>";
  message += "</body>";
  message += "</html>";

  server.send(200, "text/html", message);
}


void setup(void) {

  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);

  Serial.begin(115200);
  
  dht.begin();
  
  WiFi.begin(ssid, password);
  Serial.println();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/humidifier/1", []() {
    humidifier = HIGH;
    actionTransmitter.sendSignal(1, 'A', humidifier);
    showControlScreen();
  });

  server.on("/humidifier/0", []() {
    humidifier = LOW;
    actionTransmitter.sendSignal(1, 'A', humidifier);
    showControlScreen();
  });

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  
  server.handleClient();

  if (millis() % 4000 == 0)
    measureHumidity();
}
