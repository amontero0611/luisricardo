/*
// TCS230 color recognition sensor 
// Sensor connection pins to Arduino are shown in comments

Color Sensor      Arduino
-----------      --------
 VCC   rojo        5V
 GND   negro       GND
 s0    morado      9
 s1    verde       6
 s2    marrón      7
 s3    gris        8
 OUT   amarillo    12
 OE                GND
*/
const int s0 = 9;  
const int s1 = 6;  
const int s2 = 7;  
const int s3 = 8;  
const int out = 12;   

// Variables  
int red = 0;  
int green = 0;  
int blue = 0;  
    
void setup()   
{  
  Serial.begin(9600); 
  pinMode(s0, OUTPUT);  
  pinMode(s1, OUTPUT);  
  pinMode(s2, OUTPUT);  
  pinMode(s3, OUTPUT);  
  pinMode(out, INPUT);  
  digitalWrite(s0, HIGH);  
  digitalWrite(s1, HIGH);  
  pinMode(A1,OUTPUT);
  digitalWrite(A1, HIGH);  
  
}  
    
void loop() 
{  
  color(); 
  Serial.print("R Intensity:");  
  Serial.print(red, DEC);  
  Serial.print(" G Intensity: ");  
  Serial.print(green, DEC);  
  Serial.print(" B Intensity : ");  
  Serial.print(blue, DEC);  
  if (red < blue && red < green && red < 20)
  {  
   Serial.println(" - (Red Color)");   
  }  

  else if (blue < red && blue < green)   
  {  
   Serial.println(" - (Blue Color)");   
  }  

  else if (green < red && green < blue)  
  {  
   Serial.println(" - (Green Color)");  
  }  
  else{
  Serial.println();  
  }
  
  Serial.println();  
  delay(2000);   
  
 }  
    
void color()  
{    
  digitalWrite(s2, LOW);  
  digitalWrite(s3, LOW);  
  //count OUT, pRed, RED  
  red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH); 
  //red = map(red,65,5,0,100); 
  digitalWrite(s3, HIGH);  
  //count OUT, pBLUE, BLUE  
  blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);  
  //blue = map(blue,65,5,0,100);
  digitalWrite(s2, HIGH);  
  //count OUT, pGreen, GREEN  
  green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);  
  //green = map(green,65,5,0,100);
}
