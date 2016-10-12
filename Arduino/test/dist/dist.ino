
/*
 * Cableado sensor distancia
 * VCC gris
 * Gnd azul
 * Trig Amarillo
 * Echo blanco
 * 
 */


int distancia;
long tiempo;
int distEmisor = 4;
int distReceptor = 5;


void setup(){
  Serial.begin(9600);
  pinMode(distEmisor, OUTPUT); /*activación del pin 9 como salida: para el pulso ultrasónico*/
  pinMode(distReceptor, INPUT); /*activación del pin 8 como entrada: tiempo del rebote del ultrasonido*/
}

void loop(){
  digitalWrite(distEmisor,LOW); /* Por cuestión de estabilización del sensor*/
  delayMicroseconds(5);
  digitalWrite(distEmisor, HIGH); /* envío del pulso ultrasónico*/
  delayMicroseconds(10);
  tiempo=pulseIn(distReceptor, HIGH); /* Función para medir la longitud del pulso entrante. Mide el tiempo que transcurrido entre el envío
  del pulso ultrasónico y cuando el sensor recibe el rebote, es decir: desde que el pin 12 empieza a recibir el rebote, HIGH, hasta que
  deja de hacerlo, LOW, la longitud del pulso entrante*/
  distancia= int(0.017*tiempo); /*fórmula para calcular la distancia obteniendo un valor entero*/
  /*Monitorización en centímetros por el monitor serial*/
  Serial.println("Distancia ");
  Serial.println(distancia);
  Serial.println(" cm");
  delay(1000);
}
