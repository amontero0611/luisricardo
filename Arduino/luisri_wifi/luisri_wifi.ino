/**
 * IBM IoT Foundation managed Device
 * 
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7

//-------- Customize these values -----------
//const char* ssid = "SSID";
//const char* password = "PASSWORD";
char ssid[20];
char password[50];

#define ORG "ORG"
#define DEVICE_TYPE "ESP8266"
#define DEVICE_ID "luisri_wifi"
#define TOKEN "TOKEN"

//-------- Customize the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char publishTopic[] = "iot-2/evt/status/fmt/json";
const char commandTopic[] = "iot-2/cmd/action/fmt/json";

const char responseTopic[] = "iotdm-1/response";
const char manageTopic[] = "iotdevice-1/mgmt/manage";
const char updateTopic[] = "iotdm-1/device/update";
const char rebootTopic[] = "iotdm-1/mgmt/initiate/device/reboot";
int publishInterval;

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

String msgType;
String msgValue;

void setup() {
//configuración de wifi y conexión a IBM IoT
  
 Serial.begin(9600);

 //Retardo para permitir preparación serial
 delay(2000);
 msgType="request";
 msgValue="SSID";
 sendJSONtoSerial();//Se solicita SSID via serial
 delay(100);
 String strSsid = getStringFromSerial();
 char __ssid[strSsid.length()+1];
 strSsid.toCharArray(ssid, sizeof(__ssid));
 
 msgType="request";
 msgValue="password";
 sendJSONtoSerial();//Se solicita password via serial
 delay(100);
 String strPwd = getStringFromSerial();
 char __pwd[strPwd.length()+1];
 strPwd.toCharArray(password, sizeof(__pwd));

 msgType="request";
 msgValue="PubInt";
 sendJSONtoSerial();//Se solicita publish interval via serial
 delay(100);
 String strInt = getStringFromSerial();
 publishInterval = strInt.toInt();
 
 boolean onError=false;
  
 if (wifiConnect()) {
  //si se conecta a wifi OK
  if (mqttConnect()) { 
    //si se conecta a MQTT OK
    if (initManagedDevice()) {
      //si realiza suscripciones OK
      // setup OK. Envía mensaje via serial
      msgType = "message";
      msgValue = "OK";
      sendJSONtoSerial();
      }
    else {msgValue = "3"; onError=true;}  
    }
  else {msgValue = "2"; onError=true;}
 }
 else{msgValue = "1"; onError=true;}
 
 if (onError){
   //devuelve error vial serial
   msgType = "error";
   sendJSONtoSerial();
   while(true) {delay(60000);}  //loop infinito: error irrecuperable
 }
 
}

//fin de la configuración de wifi y conexión a IBM IoT


//--- Bucle de intercambio de mensajes entre Arduino e IBM IoT
        
void loop() {
  
        String cadena;
        while(Serial.available()>0) //Mientras haya datos en el buffer de entrada serial
            {
              delay(5); //Poner un pequeño delay para mejorar la recepción de datos
              char caracter=Serial.read();
              if (caracter != '#') 
               {cadena=String(cadena+caracter);}
              else // fin de cadena
              {
                if (publishData(cadena))  //publica la cadena
                {
                  delay(100);
                }
                else  {
                  //en caso de fallo en el envío de la cadena devuelve un error via serial
                  msgType = "error";
                  msgValue = "4";
                  sendJSONtoSerial();
                }
                break; //sale del bucle
              } //end if
             } //end while
              
 //se comprueba conexión con IoT y si hay datos esperando        
 if (!client.loop()) {
          mqttConnect();
         }
}

//---- fin del bucle


String getStringFromSerial(){  
  String cadena;
  
  while(!Serial.available()) 
  {    //se espera información via serial
  }
  while(Serial.available()>0) //Mientras haya datos en el buffer de entrada serial
  {
    delay(5); //Poner un pequeño delay para mejorar la recepción de datos
    char caracter=Serial.read();
    cadena=String(cadena+caracter);  
   } //end while
   return cadena;
}

boolean wifiConnect() {
 long initMillis=millis();
 long elapsedMillis; 
 //Serial.print("Connecting to "); Serial.print(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
 elapsedMillis=millis();
 if (elapsedMillis > (initMillis + 30000)) {return false;}
 delay(500);
 } 
 //Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
 return true;
}

boolean mqttConnect() {
 long initMillis=millis();
 long elapsedMillis; 
 if (!!!client.connected()) {
 //Serial.print("Reconnecting MQTT client to "); Serial.println(server);
 while (!!!client.connect(clientId, authMethod, token)) {
 //Serial.print(".");
 elapsedMillis=millis();
 if (elapsedMillis > (initMillis + 30000)) {return false;}
 delay(500);
 }
 //Serial.println();
 return true;
 }
}

boolean initManagedDevice() {
 //Subscribe to action commands
 if (client.subscribe(commandTopic)) {
 //Serial.println("subscribe to action commands OK");
 } else {
 //Serial.println("subscribe to action commands FAILED");
 return false;
 }
 /*  
 //Subscribe to responses
 if (client.subscribe(responseTopic)) {
 //Serial.println("subscribe to responses OK");
 } else {
 //Serial.println("subscribe to responses FAILED");
 return false;
 }
 
 //subscribe to reboot requests
 if (client.subscribe(rebootTopic)) {
 //Serial.println("subscribe to reboot OK");
 } else {
 //Serial.println("subscribe to reboot FAILED");
 return false;
 }
 */
 //subscribe to update requests
 if (client.subscribe(updateTopic)) {
 //Serial.println("subscribe to update OK");
 } else {
 //Serial.println("subscribe to update FAILED");
 return false;
 }

 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 JsonObject& d = root.createNestedObject("d");
 JsonObject& metadata = d.createNestedObject("metadata");
 metadata["publishInterval"] = publishInterval;
 JsonObject& supports = d.createNestedObject("supports");
 supports["deviceActions"] = true;

 char buff[300];
 root.printTo(buff, sizeof(buff));
 //Serial.println("publishing device metadata:"); Serial.println(buff);
 if (client.publish(manageTopic, buff)) {
 //Serial.println("device Publish ok");
 } else {  
 //Serial.print("device Publish failed:");
 return false;
 }
 
 return true;
 
}

boolean publishData(String cadena) {
 String payload = cadena;
 
 //Serial.print("Sending payload: "); Serial.println(payload);
 
 if (client.publish(publishTopic, (char*) payload.c_str())) {
 //Serial.println("Publish OK");
 return true;
 } else {
 //Serial.println("Publish FAILED");
 return false;
 }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
 char message_buff[100];
 int i = 0;
 
 //Serial.print("callback invoked for topic: "); Serial.println(topic);

 if (strcmp (commandTopic, topic) == 0) {
  
  for(i=0; i<payloadLength; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  
  //Serial.println("Received command payload: " + msgString);
  Serial.print(msgString);
  return; // just print of response for now 
  }

 /*
 if (strcmp (responseTopic, topic) == 0) {
  
  for(i=0; i<payloadLength; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  
  Serial.println("Received response payload: " + msgString);
 return; // just print of response for now 
 }

 if (strcmp (rebootTopic, topic) == 0) {
 Serial.println("Rebooting...");
 ESP.restart();
 }
 */
 if (strcmp (updateTopic, topic) == 0) {
 handleUpdate(payload); 
 } 
 
}

void handleUpdate(byte* payload) {
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)payload);
  if (!root.success()) {
   Serial.println("handleUpdate: payload parse FAILED");
   return;
  }
  Serial.println("handleUpdate payload:"); root.prettyPrintTo(Serial); Serial.println();

  JsonObject& d = root["d"];
  JsonArray& fields = d["fields"];
  for(JsonArray::iterator it=fields.begin(); it!=fields.end(); ++it) {
    JsonObject& field = *it;
    const char* fieldName = field["field"];
    if (strcmp (fieldName, "metadata") == 0) {
      JsonObject& fieldValue = field["value"];
      if (fieldValue.containsKey("publishInterval")) {
        publishInterval = fieldValue["publishInterval"];
        Serial.print("publishInterval:"); Serial.println(publishInterval);
      }
    }
  }
}

void sendJSONtoSerial(){
  
  StaticJsonBuffer<100> jsonBuffer; //size of the pool in bytes.

  // Create the root of the object tree.
  JsonObject& root = jsonBuffer.createObject();

  // Add values in the object
  
  root[msgType] = msgValue;  
  root.printTo(Serial);
   
}

