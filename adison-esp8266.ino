

/*
 * see https://github.com/AdisonTech/adison-esp8266 for documentation
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <DHT.h>

DHT dht(5, DHT22);

const char* mqtt_server = "portal.adisontech.com";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void clear_config() {
  Serial.println("Clearing EEPROM");
  WiFi.begin("hi1234", "there");
}

byte mac_[6];
char mac[14];

String topicBase;
String topicTemp;
String topicHumid;
String topicPing;

void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  topicBase = "node/" + String(mac);
  topicTemp = topicBase + "/temp";
  topicHumid = topicBase + "/humidity";
  topicPing = topicBase + "/ping";

  Serial.println(topicBase);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

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

  setup_mqtt();
}

void blink() {
  digitalWrite(BUILTIN_LED, 0);
  delay(100);
  digitalWrite(BUILTIN_LED, 1);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
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

      client.publish(topicTemp.c_str(), String(t).c_str());
      client.publish(topicHumid.c_str(), String(h).c_str());
    }
    lastMsg = now;
    ++value;
    Serial.print("Sending ping ");
    Serial.println(value);
    client.publish(topicPing.c_str(), String(value).c_str());

    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
  }
}

