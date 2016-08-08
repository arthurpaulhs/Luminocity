#include <ESP8266WiFi.h>          
#include <ESP8266WebServer.h>
#include <PubSubClient.h>         

enum {
  ACCESS_POINT_WEBSERVER = 0
};

ESP8266WebServer server(80);
// soft ap ssid and pass
const char* ssid = "ESP_ARTHUR";  
const char* passphrase = "seek9cat";

// broker information
const char* mqtt_server = "iot.eclipse.org";

//initialize pin
int ledPin1 = 14;
int ledPin2 = 16;
int sel_sensor_1 = 5;
int sel_sensor_2 = 4;
int sensor_esp = A0;
int ctrl_lamp = 12;
int ctrl_charger = 13;

//variable initialization
float current = 0;
float voltage = 0;
float buff = 0;
float var = 0;
float offset_acs_battery = 2.544;
float offset_acs_lamp = 2.500;
int voltage_c = 0;
int voltage_v = 0;
int d1, d2 = 0;
float f1 = 0;


char msg[50];
String st;
String content;
bool mark = true;

//initialize client
WiFiClient espClient;
PubSubClient client(espClient);

bool testWifi(void) {
  int c = 0;
  Serial1.println();
  Serial1.println("System now checking for available wifi credentials and attempt to connect to it for 1 minute. Please be patient.");
  Serial1.println();
  Serial1.println("WIFI STATUS VALUE");
  Serial1.println("0 : WL_IDLE_STATUS");
  Serial1.println("1 : WL_NO_SSID_AVAIL");
  Serial1.println("2 : WL_SCAN_COMPLETED");
  Serial1.println("3 : WL_CONNECTED");
  Serial1.println("4 : WL_CONNECT_FAILED");
  Serial1.println("5 : WL_CONNECTION_LOST");
  Serial1.println("6 : WL_DISCONNECTED");
  Serial1.println();
  Serial1.println("Waiting for Wifi to connect...");
  Serial1.print("WiFi status: ");
  while ( c < 60 ) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(1000);
    Serial1.print(WiFi.status());
    Serial1.print(" ");
    c++;
  }
  Serial1.println();
  Serial1.println();
  Serial1.println("Connection timed out, opening AP");
  return false;
}

void setupAccessPoint(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial1.println("Scan done");
  if (n == 0)
    Serial1.println("No networks found");
  else
  {
    Serial1.print(n);
    Serial1.println(" Networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial1.print(i + 1);
      Serial1.print(": ");
      Serial1.print(WiFi.SSID(i));
      Serial1.print(" (");
      Serial1.print(WiFi.RSSI(i));
      Serial1.print(" dB)");
      Serial1.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial1.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += " dB)";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, passphrase, 6);
  launchWeb(ACCESS_POINT_WEBSERVER);
}

void launchWeb(int webservertype) {
  setupWebServerHandlers(webservertype);
  // Start the server
  server.begin();
  Serial1.print("Server type ");
  Serial1.print(webservertype);
  Serial1.println(" (credentials configuration) has started.");
  Serial1.print("Local IP: ");
  Serial1.println(WiFi.localIP());
  Serial1.print("SoftAP IP: ");
  Serial1.print(WiFi.softAPIP());
}

void setupWebServerHandlers(int webservertype)
{
  if ( webservertype == ACCESS_POINT_WEBSERVER ) {
    server.on("/", handleDisplayAccessPoints);
    server.on("/setap", handleSetAccessPoint);
    server.onNotFound(handleNotFound);
  }
}

void handleDisplayAccessPoints() {
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String macStr = macToStr(mac);
  content = "<!DOCTYPE HTML>\n<html><h3>CREDENTIALS CONFIGURATION OF ";
  content += ssid;
  content += "<br>(";
  content += ipStr;
  content += " / ";
  content += macStr;
  content += ")</h3>";
  content += "<p>";
  content += "Available networks: ";
  content += st;
  content += "<p>Please enter credentials information on the following fields. <br>";
  content += "<form method='get' action='setap'>";
  content += "SSID: <input title='Please enter SSID from above list' input name='ssid' placeholder='SSID' length=32>";
  content += " Password: <input type='password' input title='Please enter your password' input name='pass' placeholder='PASSWORD' length=64><p>";
  content += "<button type='submit'>Save</button></form>";
  content += "<p>We will attempt to connect to the selected AP.";
  content += "</html>";
  server.send(200, "text/html", content);
}

void handleSetAccessPoint() {
  int httpstatus = 200;
  String qsid = server.arg("ssid");
  String qpass = server.arg("pass");
  if (qsid.length() > 0 && qpass.length() > 0) {
    for (int i = 0; i < qsid.length(); i++)
    {
      // Deal with (potentially) plus-encoded ssid
      qsid[i] = (qsid[i] == '+' ? ' ' : qsid[i]);
    }
    for (int i = 0; i < qpass.length(); i++)
    {
      // Deal with (potentially) plus-encoded password
      qpass[i] = (qpass[i] == '+' ? ' ' : qpass[i]);
    }
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(qsid.c_str(), qpass.c_str());
    delay(100);
    content = "<!DOCTYPE HTML>\n<html>";
    content += "Connection to access point ";
    content += qsid;
    content += " succeeded.</html>";
  } else {
    content = "<!DOCTYPE HTML><html>";
    content += "Error, no ssid or password set?</html>";
    Serial1.println("Sending 404");
    httpstatus = 404;
  }
  server.send(httpstatus, "text/html", content);
}

void handleNotFound() {
  content = "File Not Found\n\n";
  content += "URI: ";
  content += server.uri();
  content += "\nMethod: ";
  content += (server.method() == HTTP_GET) ? "GET" : "POST";
  content += "\nArguments: ";
  content += server.args();
  content += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    content += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", content);
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
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
    Serial1.println();
    Serial1.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266_Luminocity")) {
      Serial1.println("Connected to server");
      Serial1.println();
    } 
    else {
      Serial1.println();
      Serial1.print("failed, rc=");
      Serial1.println(client.state());
      Serial1.println(" try again in 5 seconds");
      Serial1.println();
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void battery_current(){ //battery current measurement
  //sensor select
  digitalWrite(sel_sensor_2, HIGH);
  digitalWrite(sel_sensor_1, HIGH);
  analogWrite(ctrl_lamp, 244);
  
  //processing data
  voltage_c = analogRead(sensor_esp);
  buff = ((voltage_c *(1.05/1023.0))*5.95);
  current = (((buff-offset_acs_battery)*1000)/185.0)*(-1000); //sensitivity 185 mV/A  
  //convert float value to int
  d1 = (int)current;
  //print data to serial
  Serial1.print(d1);
  Serial1.println(" mA");
  //publish data
  snprintf(msg,75,"Battery current = %d mA", d1);
  client.publish("luminocity/measurement_test/battery_current_measurement", msg);
  //indicator show which measurement currently executed by microprocessor
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, HIGH);
  }

void lamp_current(){ //lamp current measurement
  //sensor select
  digitalWrite(sel_sensor_2, LOW);
  digitalWrite(sel_sensor_1, HIGH);
  analogWrite(ctrl_lamp, 0);
  
  //processing data
  voltage_c = analogRead(sensor_esp);
  buff = ((voltage_c *(1.05/1023.0))*5.96);
  current = (((buff-offset_acs_lamp)*1000)/185.0)*1000; //sensitivity 185 mV/A
  //convert float value to int
  d1 = (int)current;
  //print data to serial
  Serial1.print(d1);
  Serial1.println(" mA");
  //publish data
  snprintf(msg,75,"Lamp current = %d mA", d1);
  client.publish("luminocity/measurement_test/lamp_current_measurement", msg);
  //indicator show which measurement currently executed by microprocessor
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, LOW);
  }

void battery_voltage(){ //battery voltage measurement
  //sensor select
  digitalWrite(sel_sensor_2, HIGH);
  digitalWrite(sel_sensor_1, LOW);
  
  //processing data
  voltage_v = analogRead(sensor_esp);
  buff = voltage_v *(1.05/1023.0);
  var = (buff*2.1)/3.0; 
  voltage = ((buff+var)*48); //voltage = ((voltage_v *(1.0/1023.0))*48); 
  //print data to serial
  Serial1.print(voltage);
  Serial1.println(" V");
  //convert float value to 2 value separated by point
  d1 = voltage;
  f1 = voltage - d1;
  d2 = (int)(f1*100);
  //publish data
  snprintf(msg,75,"Battery voltage = %d.%02d V", d1, d2);
  client.publish("luminocity/measurement_test/battery_voltage_measurement", msg);
  //indicator show which measurement currently executed by microprocessor
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, HIGH);
  }

void solar_panel_voltage(){ //solar panel voltage measurement
  //sensor select
  digitalWrite(sel_sensor_2, LOW);
  digitalWrite(sel_sensor_1, LOW);
  
  //processing data
  voltage_v = analogRead(sensor_esp);
  buff = voltage_v *(1.05/1023.0);
  var = (buff*2.1)/3.0; 
  voltage = ((buff+var)*48); //voltage = ((voltage_v *(1.0/1023.0))*48); 
  //print data to serial
  Serial1.print(voltage);
  Serial1.println(" V");
  //convert float value to 2 value separated by point
  d1 = voltage;
  f1 = voltage - d1;
  d2 = (int)(f1*100);
  //publish data
  snprintf(msg,75,"Solar panel voltage = %d.%02d V", d1, d2);
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
  pinMode(ctrl_lamp, OUTPUT);
  pinMode(ctrl_charger, OUTPUT);
  
  Serial1.begin(115200);
  WiFi.mode(WIFI_STA);
  if (!testWifi()) {
    setupAccessPoint(); // No WiFi yet, enter configuration mode
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  server.handleClient(); 
  if (WiFi.status() == WL_CONNECTED){
    //notify user if wifi has been connected
    if (mark == true){ 
      Serial1.println();
      Serial1.println();
      Serial1.println("WiFi connected");
      Serial1.print("Local IP: ");
      Serial1.println(WiFi.localIP());
      Serial1.print("SoftAP IP: ");
      Serial1.println(WiFi.softAPIP());
      mark = false;
    }
    
    //connectivity test
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    //measurement process
    battery_current();
    delay(500);
    lamp_current();
    delay(500);
    battery_voltage();
    delay(500);
    solar_panel_voltage();
    delay(500);
  }
}
