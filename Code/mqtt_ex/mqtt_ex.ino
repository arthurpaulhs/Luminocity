#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid        = "M2M Network";
const char* password    = "tr1tr0n1k";
const char* mqtt_server = "iot.eclipse.org";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//led on luci board
int led3 = 16;
int led4 = 14;

void setup_wifi() {
  delay(2000);
  //connecting to a WiFi network
  Serial1.println();
  Serial1.print("Connecting to ");
  Serial1.println(ssid);

  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");  
  }

  Serial1.println("");
  Serial1.println("WiFi connected");
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial1.print("Message arrived [");
  Serial1.print(topic);
  Serial1.print("] ");
  for(int i = 0; i < length; i++) {
    Serial1.print((char)payload[i]);  
  }
  Serial1.println();

  //switch led
}

void reconnect() {
  //loop until reconnect
  while(!client.connected()) {
    Serial1.print("Attempting MQTT connection");
    if(client.connect("ESPArthur")) {
      Serial1.println("connected");
      client.publish("outTopicSatu","hello world");
      client.subscribe("inTopicSatu");  
    } else {
      Serial1.print("failed, rc=");
      Serial1.print(client.state());
      Serial1.println("try again in 5 sec");
      delay(5000); 
    }
  }  
}

void setup() {
  Serial1.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);

  //setup led
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
}

void loop() {
  if(!client.connected()) {
    reconnect();  
  }
  client.loop();

  long now = millis();
  if(now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf(msg,75,"hello world #%ld", value);
    Serial1.print("Publish message: ");
    Serial1.println(msg);
    Serial1.print("RSSI: ");
    Serial1.println(WiFi.RSSI());
    client.publish("outTopicSatu",msg);  
  }
  
  digitalWrite(led3,LOW);
  digitalWrite(led4,HIGH);
  delay(250);
  digitalWrite(led3,HIGH);
  digitalWrite(led4,LOW);
  delay(250);
}
