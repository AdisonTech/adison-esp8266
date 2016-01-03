/*
 * see https://github.com/AdisonTech/adison-esp8266 for documentation
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <DHT.h>
#include <ArduinoJson.h>

DHT dht(5, DHT22);

String get_mac_address() {
  byte mac_[6];
  char mac[14];  
  
  WiFi.macAddress(mac_);
  snprintf(mac, sizeof(mac), "%x%x%x%x%x%x", 
    mac_[0],
    mac_[1],
    mac_[2],
    mac_[3],
    mac_[4],
    mac_[5],
    mac_[6]);

  return String(mac);
}

String portal_server = "mars:3000";
String site = "BEC";
String portal_api_url;

void setup_url() {
  portal_api_url = String("http://") + portal_server + String("/api/node/") + String(site) + String("/") + get_mac_address();
  Serial.println(portal_api_url);
}

void clear_config() {
  Serial.println("Clearing EEPROM");
  WiFi.begin("hi1234", "there");
}

void setup_wifi() {
  Serial.println("Setup Wifi ...");
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yey :)");
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.print("Adison Technologies ");

  Serial.print("ChipID: ");
  Serial.println(ESP.getChipId(), HEX);
  
  int key = digitalRead(BUILTIN_LED);
  Serial.print("key = ");
  Serial.println(key);

  if (key == 0) 
    clear_config();
    
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  setup_url();
  setup_wifi();
}

void blink() {
  digitalWrite(BUILTIN_LED, 0);
  delay(100);
  digitalWrite(BUILTIN_LED, 1);
}

void send_data(float t, float h) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["type"] = "esp8266";
  JsonObject& inputs = data.createNestedObject("inputs");
  inputs["temp"] = t;
  inputs["humidity"] = h;
  String data_;
  data.printTo(data_);
  Serial.println(data_);

  HTTPClient http;
  http.begin(portal_api_url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(data_);

  if (httpCode) {
    Serial.print("HTTP POST code: ");
    Serial.println(httpCode);

    if (httpCode == 200) {
      //String payload = http.getString();
      //Serial.println(payload);
    }
  } else {
    Serial.printf("HTTP POST failed\n");
  }
  http.end();
}

void get_data() {
  HTTPClient http;
  http.begin(portal_api_url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &j = jsonBuffer.parseObject(payload);
    Serial.print("GET result: ");
    j.printTo(Serial);
    Serial.println(payload);    
  } else {
    Serial.print("HTTP GET failed\n");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  long now = millis();

  blink();
  float h = dht.readHumidity();
  float t = dht.readTemperature(true);
  if (!isnan(h) && !isnan(t)) {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.println(t);

    send_data(t, h);
    get_data();
  }

  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());

  delay(1000);
}


