#include<ESP8266WiFi.h>

// variable declaration
int current_state = 2;

void setup() {
  WiFi.mode(WIFI_STA); // connection mode
  WiFi.begin("Arthur Paul","hasudungan"); //desired access point
  Serial1.begin(115200);
  delay(5000);
}

void loop() {
  if ((WiFi.waitForConnectResult() != WL_CONNECTED) && (current_state != 0)) {
    Serial1.println("Connection Failed");
    current_state = 0;
  }
  else if ((WiFi.waitForConnectResult() == WL_CONNECTED) && (current_state != 1)){
    Serial1.println("Connection Established");
    current_state = 1;
  }
}
