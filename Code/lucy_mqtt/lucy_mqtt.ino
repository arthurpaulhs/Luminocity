#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// initialize wifi and server information
const char* ssid = "mywifi2";
const char* password = "mypass2";
const char* mqtt_server = "iot.eclipse.org";

// initialize client and other variable
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// initialize led pin
int ledPin1 = 14;
int ledPin2 = 16;

void setup_wifi() { // configure wifi connection
  delay(2000);
  //connecting to a WiFi network
  Serial1.print("Attempting to connect to WPA network ");
  Serial1.print(ssid);
  WiFi.begin(ssid, password);

  // check connection
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial1.print(".");
  }
  
  Serial1.println();
  Serial1.println("WiFi connected");
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial1.print("Message arrived [");
  Serial1.print(topic);
  Serial1.print("] ");
  for (int i = 0; i < length; i++) {
    Serial1.print((char)payload[i]);
  }
  Serial1.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial1.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_Luminocity")) {
      Serial1.println("Connected to server");
      // subscribe to a topic
      client.subscribe("luminocity/test/test_subscribe");
    } 
    else {
      Serial1.print("failed, rc=");
      Serial1.println(client.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial1.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "Hello world #%ld!", value);
    Serial1.print("Publish message: ");
    Serial1.println(msg);
    Serial1.print("RSSI: ");
    Serial1.print(WiFi.RSSI());
    Serial1.println(" dB");
    client.publish("luminocity/test/test_publish",msg);
  }
  
  digitalWrite(ledPin1,LOW);
  digitalWrite(ledPin2,HIGH);
  delay(250);
  digitalWrite(ledPin1,HIGH);
  digitalWrite(ledPin2,LOW);
  delay(250);
}
