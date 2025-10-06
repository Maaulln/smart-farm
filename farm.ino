#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

// ====== Konfigurasi WiFi ======
const char* ssid = "Wifi net";
const char* password = "12345678";

// ====== Konfigurasi MQTT HiveMQ Cloud ======
const char* mqtt_server = "d835bc5132a1488a88bfed2c34dcb24a.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "maaulln";   // dari HiveMQ Access Management
const char* mqtt_pass = "Mobilelegend111";

// ====== Konfigurasi Sensor ======
#define DHTPIN 4
#define DHTTYPE DHT22
#define SOIL_PIN 34

DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure espClient;
PubSubClient client(espClient);

// ====== Fungsi koneksi WiFi ======
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

// ====== Reconnect MQTT ======
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  setup_wifi();

  // Set TLS (tanpa sertifikat CA, HiveMQ mengizinkan)
  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soil = analogRead(SOIL_PIN);

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Konversi data ke JSON
  String payload = "{\"temperature\":";
  payload += String(t);
  payload += ",\"humidity\":";
  payload += String(h);
  payload += ",\"soil\":";
  payload += String(soil);
  payload += "}";

  Serial.print("Publishing message: ");
  Serial.println(payload);

  // Publish ke topik MQTT
  client.publish("smartfarm/sensor", payload.c_str());

  delay(5000); // kirim tiap 5 detik
}
