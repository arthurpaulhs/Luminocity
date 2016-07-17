#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

enum {
  ACCESS_POINT_WEBSERVER = 0
};

ESP8266WebServer server(80);
//soft ap ssid and pass
const char* ssid = "esparthur";  
const char* passphrase = "seek9cat";
String st;
String content;

// initialize led pin
int ledPin1 = 14;
int ledPin2 = 16;

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
      Serial1.print(")");
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
    st += ")";
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
  Serial1.println();
  setupWebServerHandlers(webservertype);
  // Start the server
  server.begin();
  Serial1.print("Server type ");
  Serial1.print(webservertype);
  Serial1.println(" (credentials configuration) has started.");
  Serial1.print("Local IP: ");
  Serial1.println(WiFi.localIP());
  Serial1.print("SoftAP IP: ");
  Serial1.println(WiFi.softAPIP());
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
  content = "<!DOCTYPE HTML>\n<html>Hello from ";
  content += ssid;
  content += " at ";
  content += ipStr;
  content += " (";
  content += macStr;
  content += ")";
  content += "<p>";
  content += st;
  content += "<p><form method='get' action='setap'><label>SSID: </label>";
  content += "<input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
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
    content = "<!DOCTYPE HTML>\n<html>";
    content += "Connection to AP ";
    content += qsid;
    content += ", succedded.</html>";
    delay(5000);
    if (WiFi.status() == WL_CONNECTED) {
      Serial1.println();
      Serial1.println("WiFi connected");
      Serial1.print("Local IP: ");
      Serial1.println(WiFi.localIP());
      Serial1.print("SoftAP IP: ");
      Serial1.println(WiFi.softAPIP());
    }
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

void setup() {
  //pin mode
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  Serial1.begin(115200);
  WiFi.mode(WIFI_STA);
  if (!testWifi()) {
    setupAccessPoint(); // No WiFi yet, enter configuration mode
  }
  else{
    Serial1.println();
    Serial1.println("WiFi connected");
    Serial1.print("Local IP: ");
    Serial1.println(WiFi.localIP());
    Serial1.print("SoftAP IP: ");
    Serial1.println(WiFi.softAPIP());
  }
}

void loop() {
  server.handleClient(); 
  if (WiFi.status() == WL_CONNECTED){
    digitalWrite(ledPin1,LOW);
    digitalWrite(ledPin2,HIGH);
    delay(250);
    digitalWrite(ledPin1,HIGH);
    digitalWrite(ledPin2,LOW);
    delay(250);
  }
}
