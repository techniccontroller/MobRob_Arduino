/*
 * Created by Techniccontroller 2019
 */
#include <Arduino.h>
#include "A4988.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 8

#define DIR_VERTICAL 0
#define STEP_VERTICAL 1
#define DIR_GRIP 2
#define STEP_GRIP 3
#define SLEEP 8
#define END_GRIP 7
#define END_VERT 10

#define SERVO_PIN 9

#define DEBUG 0

A4988 stepperGRIP(200, DIR_GRIP, STEP_GRIP);
A4988 stepperVERT(200, DIR_VERTICAL,  STEP_VERTICAL);


void initGripper(){
  stepperGRIP.startRotate(-360*2);
  unsigned wait_time_micros = stepperGRIP.nextAction();
  while(wait_time_micros > 0 && digitalRead(END_GRIP) == HIGH){
    wait_time_micros = stepperGRIP.nextAction();
  }
  stepperGRIP.stop();
}

void initVertical(){
  stepperVERT.startRotate(-360*3);
  unsigned wait_time_micros = stepperVERT.nextAction();
  while(wait_time_micros > 0 && digitalRead(END_VERT) == LOW){
    wait_time_micros = stepperVERT.nextAction();
  }
  stepperVERT.stop();
}

/*void _Servo(int _angle)
{
  const int pulseabstand = 20;        // Definition der Kenngrößen
  const int Servo_minpulse = 800;     // der Servoansteuerung
  const int Servo_maxpulse = 2400;
  const int Servo_minW = 20;          // kleinster und größter Winkel
  const int Servo_maxW = 160;
  
  static unsigned long lastRefresh;
  if (millis()-lastRefresh < pulseabstand) return;      // noch keine 20 ms vergangen
  lastRefresh = millis();                               // Zeit des erfolgten Refresh merken
  
  _angle = constrain(_angle, Servo_minW, Servo_maxW);   // Stellwinkel begrenzen
  int pulselength = map(_angle, Servo_minW, Servo_maxW, Servo_minpulse, Servo_minpulse);

  noInterrupts();
  digitalWrite(SERVO_PIN,HIGH);        // Ersten Refresh-Impuls starten
  delayMicroseconds(pulselength);
  digitalWrite(SERVO_PIN,LOW);
  interrupts(); 
}*/

void _Servo(int _angle)
{
  const int pulseabstand = 20;       // Definition der Kenngrößen
  const int Servo_minpulse = 800;    // der Servoansteuerung
  const int Servo_maxpulse = 2400;
  const int Servo_minW = 20;        // kleinster und größter Winkel
  const int Servo_maxW = 160;
  // berechnen der Geradengleichung des Servos 
  const float m = ((Servo_maxpulse - Servo_minpulse)*1.0/(Servo_maxW - Servo_minW)*1.0);
  const int b = Servo_minpulse - m * Servo_minW;
  
  static unsigned long lastRefresh;
  if (millis()-lastRefresh < pulseabstand) return; // noch keine 20 ms vergangen
  lastRefresh = millis(); // Zeit des erfolgten Refresh merken
  
  _angle = constrain(_angle, Servo_minW, Servo_maxW); // Stellwinkel begrenzen
  int pulselength = _angle * m + b;
  //Serial.print("Winkel = ");Serial.print(_angle);Serial.print("   Puls: ");Serial.println(pulselength);
  noInterrupts();
  digitalWrite(SERVO_PIN,HIGH); // Ersten Refresh-Impuls starten
  delayMicroseconds(pulselength);
  digitalWrite(SERVO_PIN,LOW);
  interrupts(); 
}  // end ServoRefresh

void setup() {
  pinMode(SLEEP, OUTPUT);
  pinMode(END_GRIP, INPUT_PULLUP);
  pinMode(END_VERT, INPUT_PULLUP);
  digitalWrite(SLEEP, HIGH);
  stepperGRIP.begin(RPM, MICROSTEPS);
  stepperVERT.begin(RPM, MICROSTEPS);
  
  initVertical();
  initGripper();

  digitalWrite(SLEEP, LOW);
  
  pinMode(SERVO_PIN, OUTPUT);
}

long lasttime = millis();
int winkel = 50;

void loop() {
  
  /*
   * Moving motor one full revolution using the degree notation
   */
  //stepperGRIP.rotate(360);
  //stepperVERT.rotate(360);
  
  
  /*
   * Moving motor to original position using steps
   */
  //stepperGRIP.move(-MOTOR_STEPS*MICROSTEPS);
  //stepperVERT.move(-MOTOR_STEPS*MICROSTEPS);
  
  // pause and allow the motor to be moved by hand
  // stepper.disable();
  
  if(millis()-lasttime > 2000){
    if(winkel == 80){
      winkel = 140;
    }
    else{
      winkel = 80;
    }
    lasttime = millis();
  }

  _Servo(winkel);
  
  delay(10);
}
