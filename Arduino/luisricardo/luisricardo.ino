#include <SoftwareSerial.h>  
#include <ServoTimer2.h>      //ServoTimer2 en lugar de Servo para prevenir interferencias con SoftwareSerial
//#include <TimedAction.h>

// ************************************************
// * establecer true para debug                   *
// ************************************************
boolean debug=false;   

//***********  valores de conexión wifi
String wifiCCSID = "Angel GS7";
String wifiPwd = "p0ughkeeps1e";
//String wifiCCSID = "Casa";
//String wifiPwd = "WQLAT8BGGROGYSOIGUS365DSGSE";
String publishInterval = "3000"; 

SoftwareSerial WF1(3, 2); // RX | TX
byte resetPin=A0;

//*********** led de estado de conexión
byte ledRojo = A2;
byte ledVerde = A3;

//*********** valores de manejo de servos
int pinI = 11;
int pinD = 10;
int stop_position = 1500;  //ancho de pulso en microsegundos para parada del servo FS5106R
int velocity = 150;    //microsegundos para sumar a la posicin de parada y establecer velocidad
int velocityTurn = 150;
int velocityBack = 150;

ServoTimer2 servoD;
ServoTimer2 servoI;

boolean mustRead=false;
boolean movingForward=false;
boolean movingBackwards=false;


//*********** valores del sensor de color
/*
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
const int S0 = 9;  
const int S1 = 6;  
const int S2 = 7;  
const int S3 = 8;  
const int sensorOut = 12;   

//*********** valores del sensor de distancia
/*
 * Cableado sensor distancia
 * VCC gris
 * Gnd azul
 * Trig Amarillo
 * Echo blanco
 * 
 */
boolean mustCheckDistance = true;  //establecer true para medir distancia y detener el robot por debajo de la minima
int distEmisor = 4;
int distReceptor = 5;
int minDistance = 15; //distancia minima libre al frente en cm
int lectArray[3];    //array de lecturas
int lectCount = 0;  //indice del array de lecturas

//********************************************************************
//*********** ejecución de funciones por intervalo de tiempo *********
//********************************************************************
/*
void getAndExecCommand();
void getColors();
void checkDistance();

TimedAction getAndExecCommandAction = TimedAction(20,getAndExecCommand);
TimedAction getColorsAction = TimedAction(3000,getColors);
TimedAction checkDistanceAction = TimedAction(100,checkDistance);
*/
int timeCommand;
int commandExecInterval = 20;
int timeColors;
int getColorsInterval = 3000;
int timeDistance;
int checkDistanceInterval = 100;


// ***************************************
// ***************  SETUP  ***************
// ***************************************

void setup()
  { 
    if (debug) {Serial.begin(9600);}
    WF1.begin(9600);  
    
    pinMode(resetPin, OUTPUT);
    pinMode(ledRojo, OUTPUT);
    pinMode(ledVerde, OUTPUT);
    digitalWrite(ledRojo, HIGH);digitalWrite(ledVerde, LOW);  //led rojo
    
    //prepara el sensor de color
    pinMode(S0, OUTPUT);  
    pinMode(S1, OUTPUT);  
    pinMode(S2, OUTPUT);  
    pinMode(S3, OUTPUT);  
    pinMode(sensorOut, INPUT);  
    /* 
     *  Frequency scaling
     *  s0 s1  Scale
     *  L  L   OFF
     *  L  H   2%
     *  H  L   20%
     *  H  H   100%
     */      
    // Setting frequency-scaling to 100%
    digitalWrite(S0,HIGH);
    digitalWrite(S1,HIGH);

    //prepara el sensor de distancia
    if(mustCheckDistance) {
      pinMode(distEmisor, OUTPUT); 
      pinMode(distReceptor, INPUT); 
      initDistanceSensor();
    }

    // conexión a bluemix
    getConnection(); 

    timeDistance = millis();
  }  //end setup



// ***************************************  
// ***************  LOOP  ****************
// ***************************************

void loop()
  {  
    /*
    checkDistanceAction.check();
    getAndExecCommandAction.check();
    getColorsAction.check();
    */
    int curMillis = millis();
    if (mustCheckDistance && ((curMillis - timeDistance) > checkDistanceInterval)) {
      checkDistance();
      timeDistance = millis();     
    }

    if ((curMillis - timeCommand) > commandExecInterval) {
      getAndExecCommand();
      timeCommand = millis();
    }

    if ((curMillis - timeColors) > getColorsInterval) {
      getColors();
      timeColors = millis();
    }
    
  }

//********** end loop ********************




//************** conexión a bluemix
void getConnection()
  {
   String receivedStr; 

   //rearranca módulo wifi
   resetWifi();

   //espera a que el wifi envíe algún dato
   while(!WF1.available())  
    {    
    }
   delay(2000); //espera a que el wifi termine de escribir info de arranque

    /*
    * Establecimiento de conexión a Bluemix
    */
   boolean connected = false;

   while (!connected) {
      
        //lee cadena que proviene del módulo wifi
        receivedStr = getBracketedStringFromWifi();
    
        if (receivedStr == "{\"request\":\"SSID\"}"){
          if (debug) {Serial.println("recibido " + receivedStr);}
          sendStringToWifi(wifiCCSID);
        }
        if (receivedStr == "{\"request\":\"password\"}"){
          if (debug) {Serial.println("recibido " + receivedStr);}
          sendStringToWifi(wifiPwd);
        }
        if (receivedStr == "{\"request\":\"PubInt\"}"){
          if (debug) {Serial.println("recibido " + receivedStr);}
          sendStringToWifi(publishInterval);
        }
        
        if (receivedStr.substring(2,7) == "error"){
          // error de conexión, resetear wifi y recomenzar
          if (debug) {Serial.println("recibido " + receivedStr);}
          resetWifi();
          //espera a que el wifi escriba algo en el serial
          while(!WF1.available())           
          {    //se espera información via serial
          }
          delay(2000); //espera a que el wifi termine de escribir info de arranque
        }
        
        if (receivedStr == "{\"message\":\"OK\"}"){
          // conectado, terminado el setup
          if (debug) {Serial.println("recibido " + receivedStr);}
          digitalWrite(ledRojo, LOW);digitalWrite(ledVerde, HIGH); // led verde
          connected = true;
        }
   }

}

//************** rearranca módulo wifi
void resetWifi()
{
//rearranca el módulo wifi  
  digitalWrite(ledRojo, HIGH);digitalWrite(ledVerde, LOW);  //led rojo
  
  digitalWrite(resetPin, HIGH); // reseteo wifi mediante un mosfet conectado a resetPin
  if (debug) {Serial.println("resetting...");}
  delay(200);
  digitalWrite(resetPin, LOW);
}

//************** recupera del wifi cadena entre llaves {}
String getBracketedStringFromWifi(){  

  String cadena;
  char caracter;
  while(WF1.available())
  {
    caracter=WF1.read();  
    if (caracter == '{') {
      cadena = String(cadena+caracter);
      break;
    }
  }
  while(WF1.available())
  {
    caracter=WF1.read();
    cadena=String(cadena+caracter); 
    if (caracter == '}') {
      break; 
    }
   } //end while
   return cadena;
}


//******************  Envía cadena al módulo wifi
void sendStringToWifi(String cadena) {
  int lenCadena = cadena.length();
  int i;
  if (debug) {Serial.print("enviando ");Serial.println(cadena);}
  for(i=0; i<=lenCadena; i++) {
     WF1.print(cadena.charAt(i));
  }
  
}


//****************** acción de recibir y ejecutar comando
void getAndExecCommand() {

String receivedStr; 

//Sólo si se recibe algo desde el wifi
if(WF1.available())  
    {    
      receivedStr = getBracketedStringFromWifi();
      if (receivedStr == "{\"command\":\"forward\"}"){
          if (debug) {Serial.println("recibido " + receivedStr);}
          if(mustCheckDistance) {
            //reinicia datos de distancia
            initDistanceSensor();
          }  
          //ejecuta comando forward
          goForward();
        }
      if (receivedStr == "{\"command\":\"back\"}"){
          if (debug) {Serial.println("recibido " + receivedStr);}
          //ejecuta comando back
          goBackwards();
        }
      if (receivedStr == "{\"command\":\"left\"}"){
          if (debug) {Serial.println("recibido " + receivedStr);}
          //ejecuta comando left
          turnLeft();
        }
      if (receivedStr == "{\"command\":\"right\"}"){
          if (debug) {Serial.println("recibido " + receivedStr);}
          //ejecuta comando right
          turnRight();
        }
      if (receivedStr == "{\"command\":\"stop\"}"){
          if (debug) {Serial.println("recibido " + receivedStr);}
          //ejecuta comando stop
          stopMoving();
        }
      if (receivedStr.substring(2,7) == "error"){
          // error de conexión, resetear wifi y recomenzar
          if (debug) {Serial.println("recibido " + receivedStr);}
          getConnection(); //conexión a bluemix
        }  
    }  
}

//***********************************************
// funciones de lectura de color
//***********************************************
void getColors() {
  if (mustRead) {
    String msgRed = getRed();
    String msgGreen = getGreen();
    String msgBlue = getBlue();
    String msgColors = "{\"d\": [" + msgRed + "," + msgGreen + "," + msgBlue + "]}#";  // '#' fin de cadena
    sendStringToWifi(msgColors);
  }
}
String getRed() {
  unsigned long tiempo;
  unsigned long duracion;
  int frequency = 0;
  String eventMsg = "{\"R\":";
  // Setting red filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  // Reading the output frequency
  //frequency = pulseIn(sensorOut, LOW);
  
  // *** lógica alternativa al uso de pulseIn() para evitar la interferencia de interrupciones 
  duracion = 0;
  tiempo=micros(); 
  while (digitalRead(sensorOut)==HIGH && duracion < 1000){
        //espera a que el pin se ponga a LOW
        duracion=micros()-tiempo; //duración del pulso
  }
  duracion = 0;
  tiempo=micros(); //pin LOW, empieza a contar el tiempo
  while (digitalRead(sensorOut)==LOW && duracion < 1000){
        //espera a LOW de nuevo durante un máximo de 5 milisecs
  duracion=micros()-tiempo; //duración del pulso
  }
  // *** end pulseIn()

  frequency=map(duracion,65,5,0,100);
  if (frequency<0) {frequency=0;}
  if (frequency>100) {frequency=100;}
  //sends event msg to wifi
  eventMsg = eventMsg + frequency + "}";
  return eventMsg;
  //sendStringToWifi(eventMsg);
}

String getGreen() {
  unsigned long tiempo;
  unsigned long duracion;
  int frequency = 0;
  String eventMsg = "{\"G\":";
  // Setting Green filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  //frequency = pulseIn(sensorOut, LOW);
  // *** lógica alternativa al uso de pulseIn() para evitar la interferencia de interrupciones 
  duracion = 0;
  tiempo=micros(); 
  while (digitalRead(sensorOut)==HIGH  && duracion < 1000){
        //espera a que el pin se ponga a LOW
        duracion=micros()-tiempo; //duración del pulso
  }
  duracion = 0;
  tiempo=micros(); //pin LOW, empieza a contar el tiempo
  while (digitalRead(sensorOut)==LOW && duracion < 1000){
        //espera a LOW de nuevo durante un máximo de 5 milisecs
  duracion=micros()-tiempo; //duración del pulso
  }
  // *** end pulseIn()
  
  frequency=map(duracion,65,5,0,100);
  if (frequency<0) {frequency=0;}
  if (frequency>100) {frequency=100;}
  //sends event msg to wifi
  eventMsg = eventMsg + frequency + "}";
  return eventMsg;
  //sendStringToWifi(eventMsg);
}  

String getBlue() {
  unsigned long tiempo;
  unsigned long duracion;
  int frequency = 0;
  String eventMsg = "{\"B\":";
  // Setting Blue filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  //frequency = pulseIn(sensorOut, LOW);
  // *** lógica alternativa al uso de pulseIn() para evitar la interferencia de interrupciones 
  duracion = 0;
  tiempo=micros(); 
  while (digitalRead(sensorOut)==HIGH && duracion < 1000){
        //espera a que el pin se ponga a LOW
        duracion=micros()-tiempo; //duración del pulso
  }
  duracion = 0;
  tiempo=micros(); //pin LOW, empieza a contar el tiempo
  while (digitalRead(sensorOut)==LOW && duracion < 1000){
        //espera a LOW de nuevo durante un máximo de 5 milisecs
  duracion=micros()-tiempo; //duración del pulso
  }
  // *** end pulseIn()
  frequency=map(duracion,65,5,0,100);
  if (frequency>100) {frequency=100;}
  if (frequency<0) {frequency=0;}
  //sends event msg to wifi
  eventMsg = eventMsg + frequency + "}";
  return eventMsg;
  //sendStringToWifi(eventMsg);
}  

//***********************************************
// obtención de distancia libre al frente
// se comprueban dos valores de cada tres usando un array
// para evitar falsos positivos
//***********************************************
void checkDistance() {  
  int distancia;
  int i;
  int positivos = 0;
  if(movingForward && mustCheckDistance) {
      distancia = getDistance();
      
      lectArray[lectCount] = distancia;
      lectCount++;if (lectCount == 3) {lectCount=0;}    
    
      //comprueba que al menos 2 de las 3 ultimas lecturas son positivas
      for (i=0;i<3;i++){
        if (debug) {
          Serial.print("Array-");Serial.print(i);Serial.print(" = ");Serial.println(lectArray[i]);
        }
        if (lectArray[i] < minDistance) {
          positivos ++;
        }
      }
      if (positivos >= 2) {
        if (debug) {Serial.println("stopMoving()");}
        stopMoving();
      }
   }
}

// lectura unitaria de distancia
int getDistance() {
 int distancia;
 unsigned long tiempo;
 unsigned long duracion;

 digitalWrite(distEmisor,LOW); /* Para estabilización del sensor*/
 waitMicrosecs(5);  //alternativo a delayMicrosecs() para evitar interferencia en interrupciones
 digitalWrite(distEmisor, HIGH); /* envío de un pulso limpio de 10 us*/
 waitMicrosecs(10);
 digitalWrite(distEmisor,LOW);
 // *** lógica alternativa al uso de pulseIn() para evitar la interferencia de interrupciones 
 duracion = 0;
 tiempo=micros(); 
 while (digitalRead(distReceptor)==LOW){
        //espera a que el pin se ponga a HIGH
        duracion=micros()-tiempo; //duración del pulso
        if (duracion > 5000) {return 100;}   //después de un timeout de 5 milisecs devuelve una distancia de 100
 }
 duracion = 0;
 tiempo=micros(); //pin HIGH, empieza a contar el tiempo
 while (digitalRead(distReceptor)==HIGH && duracion < 5000){
        //espera a LOW de nuevo durante un máximo de 5 milisecs
 duracion=micros()-tiempo; //duración del pulso
 }
 // *** end pulseIn()
 distancia= int(0.017*duracion); /*fórmula para calcular la distancia en cm*/
 return distancia;
}

//********** inicializa lecturas del sensor de distancia
void initDistanceSensor() {
  // rellena el array de lecturas con tres lecturas iniciales.
  // se evaluara un minimo de dos positivos entre tres
  // para evitar falsos positivos

  int distancia;
    
    for (lectCount=0;lectCount<3;lectCount++) {
       distancia = getDistance(); 
       lectArray[lectCount]= distancia;
       if (debug) {
         Serial.print("Array-");Serial.print(lectCount);Serial.print(" = ");Serial.println(lectArray[lectCount]);
       }  
    }
    lectCount=0;
}

//alternativo a delayMicrosecs() para evitar interferencia en interrupciones
void waitMicrosecs(int waitMicrosecs) {
  unsigned long initMicrosecs = micros();
  while (micros() < (initMicrosecs + waitMicrosecs)) {}  //espera sin hacer nada
}



//***********************************************
// funciones de movimiento
//***********************************************
void goForward() {
  stopMoving();
  //conecta los servos
  servoD.attach(pinD);
  servoI.attach(pinI);
  
  movingForward=true;
  mustRead=true;
  
  servoD.write(stop_position-velocity);
  servoI.write(stop_position+velocity);
}

void stopMoving() {
  mustRead=false;
  movingForward=false;
  movingBackwards=false;
  servoD.write(stop_position);
  servoI.write(stop_position);
  //desconecta los servos
  servoD.detach();
  servoI.detach();
}

void turnRight() {
  stopMoving();
  //conecta los servos
  servoD.attach(pinD);
  servoI.attach(pinI);
  servoD.write(stop_position+velocityTurn);
  servoI.write(stop_position+velocityTurn);
}

void turnLeft() {
  stopMoving();
  //conecta los servos
  servoD.attach(pinD);
  servoI.attach(pinI);
  servoD.write(stop_position-velocityTurn);
  servoI.write(stop_position-velocityTurn);
}

void goBackwards() {
  stopMoving();
  //conecta los servos
  servoD.attach(pinD);
  servoI.attach(pinI);
  
  movingBackwards=true;
  mustRead=true;
  
  servoD.write(stop_position+velocityBack);
  servoI.write(stop_position-velocityBack);
}


