#include<ESP8266WiFi.h>

void setup() {
  Serial1.begin(115200); // baud rate 
  WiFi.mode(WIFI_AP); // connection mode
  WiFi.softAP("ESP_Test","seek9cat"); //ssid and password generation
}

void loop() {
  // put your main code here, to run repeatedly:

}
