/*
 * Created by Techniccontroller 2019
 */
#include <Arduino.h>
#include "A4988.h"
#include "TinyServo.h"

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
#define RESET 7
#define SLEEP 8
#define END_GRIP 5
#define END_VERT 10

#define SERVO_PIN 9

#define DEBUG 0

A4988 stepperGRIP(200, DIR_GRIP, STEP_GRIP);
A4988 stepperVERT(200, DIR_VERTICAL,  STEP_VERTICAL);
TinyServo servo(SERVO_PIN);

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

void setup() {
  pinMode(RESET, OUTPUT);
  pinMode(SLEEP, OUTPUT);
  pinMode(END_GRIP, INPUT_PULLUP);
  pinMode(END_VERT, INPUT_PULLUP);
  digitalWrite(RESET, HIGH);
  digitalWrite(SLEEP, HIGH);
  stepperGRIP.begin(RPM, MICROSTEPS);
  stepperVERT.begin(RPM, MICROSTEPS);
  
  initVertical();
  initGripper();
  
  servo.write(90);
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
  
  /*if(millis()-lasttime > 2000){
    if(winkel == 80){
      winkel = 140;
    }
    else{
      winkel = 80;
    }
    lasttime = millis();
    servo.write(winkel);
  }*/

  servo.refresh();
  
  delay(10);
}
