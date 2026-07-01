#include <WiFi.h>
#include <ESPping.h>    // Library by dvarrel
#include <PubSubClient.h>

// --- Configuration ---
const char* ssid = "realme narzo 60x 5G";
const char* password = "n83c8y8n";
const char* mqtt_server = "10.238.152.2"; // e.g., "192.168.1.50"
const int mqtt_port       = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned long length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("esp32/commands");
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
  setup_wifi();

  // --- Ping Test Implementation ---
  Serial.print("Pinging Broker at ");
  Serial.println(mqtt_server);
  
  if(Ping.ping(mqtt_server, 3)) {
    Serial.println("Ping Success! Network path is clear.");
    Serial.print("Average response time: ");
    Serial.print(Ping.averageTime());
    Serial.println(" ms");
  } else {
    Serial.println("Ping FAILED. Check Pi IP and Router settings.");
  }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Publish a heartbeat every 10 seconds
  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    String msg = "ESP32 Uptime: " + String(now / 1000) + "s";
    Serial.print("Publishing: ");
    Serial.println(msg);
    client.publish("esp32/data", msg.c_str());
  }
}