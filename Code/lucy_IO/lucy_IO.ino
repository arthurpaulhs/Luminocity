int ledPin = 16;

void setup() {
  // Program Setup
  Serial1.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Main Program
  digitalWrite(ledPin,HIGH);
  Serial1.println("LED Status: ON");
  delay(1000);
  digitalWrite(ledPin, LOW);
  Serial1.println("LED Status: OFF");
  delay(1000);
}
