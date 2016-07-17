#include <FS.h> 
#include <ESP8266WiFi.h>          
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>         

//initialize pin
int ledPin1 = 14;
int ledPin2 = 16;
int sel_sensor_1 = 5;
int sel_sensor_2 = 4;
int sensor_esp = A0;

//variable initialization
float current = 0;
float voltage = 0;
int voltage_c = 0;
int voltage_v = 0;
char msg[50];

//initialize client
WiFiClient espClient;
PubSubClient client(espClient);

//mqtt server and port decaration
char mqtt_server[40];
char mqtt_port[6];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save configuration
void saveConfigCallback () {
  Serial1.println("New configuration detected");
  shouldSaveConfig = true;
}

//mqtt callback, notify when there is new messagge at subsciribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial1.print("Message arrived [");
  Serial1.print(topic);
  Serial1.print("] ");
  for (int i = 0; i < length; i++) {
    Serial1.print((char)payload[i]);
    msg[i] = (char)payload[i];
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

void battery_current(){ //battery current measurement
  //sensor select
  digitalWrite(sel_sensor_2, HIGH);
  delay(50);
  digitalWrite(sel_sensor_1, HIGH);
  
  //processing data
  voltage_c = analogRead(sensor_esp);
  current = (((voltage_c *(1.0/1023.0))-(5.0/9.0))*67500);    
  //publish data
  Serial1.println(current);
  snprintf (msg, 75, "Battery current = %.2f mA", current);
  client.publish("luminocity/measurement_test/battery_current_measurement", msg);
  //indicator show which measurement currently executed by microprocessor
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, HIGH);
  delay(250);
  }

void lamp_current(){ //lamp current measurement
  //sensor select
  digitalWrite(sel_sensor_2, LOW);
  delay(50);
  digitalWrite(sel_sensor_1, HIGH);
  
  //processing data
  voltage_c = analogRead(sensor_esp);
  current = (((voltage_c *(1.0/1023.0))-(5.0/9.0))*67500);
  //publish data
  Serial1.println(current);
  snprintf (msg, 75, "Lamp current = %.2f mA", current);
  client.publish("luminocity/measurement_test/lamp_current_measurement", msg);
  //indicator show which measurement currently executed by microprocessor
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, LOW);
  }

void battery_voltage(){ //battery voltage measurement
  //sensor select
  digitalWrite(sel_sensor_2, HIGH);
  delay(50);
  digitalWrite(sel_sensor_1, LOW);
  
  //processing data
  voltage_v = analogRead(sensor_esp);
  voltage = ((((voltage_v *(1.0/1023.0))*10)/11)*48);   
  //publish data
  Serial1.println(voltage);
  snprintf (msg, 75, "Battery voltage = %5.2f V", voltage);
  client.publish("luminocity/measurement_test/battery_voltage_measurement", msg);
  //indicator show which measurement currently executed by microprocessor
  //digitalWrite(ledPin1, LOW);
  //digitalWrite(ledPin2, HIGH);
  }

void solar_panel_voltage(){ //solar panel voltage measurement
  //sensor select
  digitalWrite(sel_sensor_2, LOW);
  delay(50);
  digitalWrite(sel_sensor_1, LOW);
  
  //processing data
  voltage_v = analogRead(sensor_esp);
  voltage = ((((voltage_v *(1.0/1023.0))*10)/11)*48);   
  //publish data
  Serial1.println(voltage);
  snprintf (msg, 75, "Solar panel voltage = %5.2f V", voltage);
  client.publish("luminocity/measurement_test/solar_panel_voltage_measurement", msg);
  //indicator show which measurement currently executed by microprocessor
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  }
  
void setup() {
  //pin mode
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(sel_sensor_1, OUTPUT);
  pinMode(sel_sensor_2, OUTPUT);
  pinMode(sensor_esp, INPUT);
  
  Serial1.begin(115200);
  Serial1.println();

  //read configuration from file system json
  Serial1.println("Mounting file system...");

  if (SPIFFS.begin()) {
    Serial1.println("File system mounted ");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial1.println();
      Serial1.println("Reading configuration file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial1.println("Opened configuration file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial1.println("\nParsed json");
          Serial1.println();
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
        } 
        else {
          Serial1.println("Failed to load json config");
        }
      }
    }
  } else {
    Serial1.println("Failed to mount file system");
  }
  //end read

  //define costum parameter
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);

  //local intialization
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set custom ip for portal
  wifiManager.setAPStaticIPConfig(IPAddress(19,98,14,2), IPAddress(19,98,14,0), IPAddress(255,0,0,0));
  
  //add new parameters
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimum quality of signal so it ignores AP's under that quality
  wifiManager.setMinimumSignalQuality(15);
  
  //set timeout until configuration portal gets turned off
 wifiManager.setTimeout(600);

  //fetches ssid and pass and tries to connect
  if (!wifiManager.autoConnect("ESP_Arthur", "seek9cat")) {
    Serial1.println("Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  
  //save the custom parameters to file systems
  if (shouldSaveConfig) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial1.println("Failed to open configuration file for writing");
    }
    Serial1.println("Configuration saved");
    json.printTo(Serial1);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  //connected to the WiFi
  Serial1.println();
  Serial1.println("WiFi connected");

  //mqtt setup
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(callback);
} 

void loop() {
    //connectivity test
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    //measurement process
    //battery_current();
    //delay(2000);
    //lamp_current();
    //delay(2000);
    battery_voltage();
    delay(2000);
    solar_panel_voltage();
    delay(2000);
}
