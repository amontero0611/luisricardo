/*
 * Servomotor 360 degree
 *
 * Sample code to control a servo motor 360 degrees.
 */

#include <Servo.h>

int pinI = 11;
int pinD = 10;
int stop_position = 93;
int velocity = 15;
int velocityTurn = 10;
int velocityBack = 10;

Servo servoD;
Servo servoI;

void setup() {
  servoD.attach(pinD);
  servoI.attach(pinI);

  goForward();

  delay(2000);

  turnRight();

  delay(2000);

  goForward();
  
  delay(2000);

  turnLeft();

  delay(2000);

  goForward();

  delay(2000);
  
  stopMoving();

  goBackwards();

  delay(2000);
  
  stopMoving();
}

void loop() {

}


void goForward() {
  servoD.write(stop_position-velocity);
  servoI.write(stop_position+velocity);
}

void stopMoving() {
  servoD.write(stop_position);
  servoI.write(stop_position);
}

void turnRight() {
  stopMoving();
  servoD.write(stop_position+velocityTurn);
  servoI.write(stop_position+velocityTurn);
}

void turnLeft() {
  stopMoving();
  servoD.write(stop_position-velocityTurn);
  servoI.write(stop_position-velocityTurn);
}

void goBackwards() {
  servoD.write(stop_position+velocityBack);
  servoI.write(stop_position-velocityBack);
}
