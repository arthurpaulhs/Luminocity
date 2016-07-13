// library inclusion
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// wifi information
const char* ssid = "Arthur Paul";
const char* password = "hasudungan";

// server information
const char* host = "http://arthur.site88.net";
String path ="http://arthur.site88.net/light.json";

// initialize led pin and ip adddress
int ledPin = 14;
IPAddress ip;

void setup() {
  // serial communication setup
  Serial1.begin(115200);

  // initialize pin
  pinMode(ledPin, OUTPUT);
  delay(2000);

  // initialize connection
  Serial1.print("Attempting to connect to WPA network ");
  Serial1.print(ssid);
  WiFi.begin(ssid, password);

  // check connection
  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    delay(1000);
    Serial1.print(".");
  }
  Serial1.println();
  Serial1.println("Connected to WiFi");
  ip = WiFi.localIP();
  Serial1.print("IP address: ");
  Serial1.println(ip);
  Serial1.println();
}

void loop() {
  Serial1.print("Connecting to ");
  Serial1.println(host);

  // variable declaration
  WiFiClient client;
  const int httpPort = 80;

  // start connection with server
 if (!client.connect(host, httpPort)) {
    Serial1.println("Unable to connect");
    delay(1000);
    return;
 }

 // sending request to server
 client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: keep-alive\r\n\r\n");

  delay(500); // wait for server to respond
  
  // read response
  String section="header";
  while(client.available()){
    String line = client.readStringUntil('\r');
    if (section=="header") { // headers
      if (line=="\n") { // skips the empty space at the beginning 
        section="json";
      }
    }
    else if (section=="json") {  // print the good stuff
      section="ignore";
      String result = line.substring(1);

      // Parse JSON
      int size = result.length() + 1;
      char json[size];
      result.toCharArray(json, size);
      StaticJsonBuffer<200> jsonBuffer; // allocate memory (byte) for incoming response
      JsonObject& json_parsed = jsonBuffer.parseObject(json);
      if (!json_parsed.success()) //check parsing proccess
      {
        Serial1.println("parseObject() failed");
        return;
      }

      // Make the decision to turn off or on the LED
      if (strcmp(json_parsed["light"], "on") == 0) {
        digitalWrite(ledPin, HIGH); 
        Serial1.println("LED status: ON");
      }
      else {
        digitalWrite(ledPin, LOW);
        Serial1.println("LED status: OFF");
      }
    }
  }
  Serial1.println("Closing Connection. ");
  Serial1.println();
}
