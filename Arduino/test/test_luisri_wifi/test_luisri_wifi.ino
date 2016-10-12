#include <SoftwareSerial.h>
SoftwareSerial WF1(3, 2); // RX | TX

byte resetPin=A0;
void setup()
  {  
    delay(3000);
    pinMode(resetPin, OUTPUT);
    resetWifi();
    Serial.begin(9600);
    WF1.begin(9600);
  }

void loop()
  {  String B= "." ;
     if (WF1.available())
         { char c = WF1.read() ;
           Serial.print(c);
         }
     if (Serial.available())
         {  char c = Serial.read();
         if (c == '#') { resetWifi();}
         else {
            WF1.print(c);
              }
         }
   }

void resetWifi()
{
  digitalWrite(resetPin, HIGH);
  Serial.println("resetting...");
  delay(200);
  digitalWrite(resetPin, LOW);
}

