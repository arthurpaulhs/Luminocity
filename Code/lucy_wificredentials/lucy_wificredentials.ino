#include <ESP8266WiFi.h>          
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>        

// initialize led pin
int ledPin1 = 14;
int ledPin2 = 16;

int i;

void setup() {
    // pin mode
    pinMode(ledPin1, OUTPUT);
    pinMode(ledPin2, OUTPUT);

    // local intialization
    WiFiManager wifiManager;
    
    // reset saved settings
    wifiManager.resetSettings();
    
    // set custom ip for portal
    wifiManager.setAPStaticIPConfig(IPAddress(19,98,14,2), IPAddress(19,98,14,0), IPAddress(255,0,0,0));

    // fetches ssid and pass from eeprom and tries to connect
    // if it does not connect it starts an access point with the specified name
    // and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("AutoConnectAP_Arthur", "seek9cat");
    
    for(i=0;i=100;i++){
      digitalWrite(ledPin1,LOW);
      digitalWrite(ledPin2,HIGH);
      delay(250);
      digitalWrite(ledPin1,HIGH);
      digitalWrite(ledPin2,LOW);
      delay(250);
    }
} 

void loop() {
    // put your main code here, to run repeatedly:
    
}
