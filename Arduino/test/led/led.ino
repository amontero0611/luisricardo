
byte ledRojo = A2;
byte ledVerde = A3;

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(ledRojo, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(ledVerde, LOW);
  delay(1000);              // wait for a second
  digitalWrite(ledRojo, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(ledVerde, HIGH);
  delay(1000);              // wait for a second
}
