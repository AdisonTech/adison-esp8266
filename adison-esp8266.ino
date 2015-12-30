



/*
 * see https://github.com/AdisonTech/adison-esp8266 for documentation
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

const char* mqtt_server = "portal.adisontech.com";

WiFiClient espClient;

long lastMsg = 0;
char msg[50];
int value = 0;

void clear_config() {
  Serial.println("Clearing EEPROM");
  WiFi.begin("hi1234", "there");
}

byte mac_[6];
char mac[14];

StaticJsonBuffer<200> jsonBuffer;

void json_test() {
  Serial.println("json_test");
  JsonObject& root = jsonBuffer.createObject();
  root["sensor"] = "gps";
  root["time"] = 1351824120;
  JsonArray& data = root.createNestedArray("data");
  data.add(48.756080, 6);  // 6 is the number of decimals to print
  data.add(2.302038, 6);   // if not specified, 2 digits are printed
  Serial.println("JSON Buffer output: ");
  root.printTo(Serial);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  json_test();

  WiFi.macAddress(mac_);
  snprintf(mac, sizeof(mac), "%x%x%x%x%x%x", 
    mac_[0],
    mac_[1],
    mac_[2],
    mac_[3],
    mac_[4],
    mac_[5],
    mac_[6]);
  
  Serial.print("Adison Technologies ");
  Serial.println(mac);

  Serial.print("ChipID: ");
  Serial.println(ESP.getChipId(), HEX);
  
  int key = digitalRead(BUILTIN_LED);
  Serial.print("key = ");
  Serial.println(key);

  if (key == 0) 
    clear_config();
    
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");
}

void blink() {
  digitalWrite(BUILTIN_LED, 0);
  delay(100);
  digitalWrite(BUILTIN_LED, 1);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {    
    blink();
    float h = dht.readHumidity();
    float t = dht.readTemperature(true);
    if (!isnan(h) && !isnan(t)) {
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.println(t);
    }

    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
  }
}


