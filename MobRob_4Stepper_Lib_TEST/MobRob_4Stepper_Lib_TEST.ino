#include "A4988.h"

#define DIR_RIGHT_FRONT 14
#define STEP_RIGHT_FRONT 15

#define DIR_LEFT_FRONT 16
#define STEP_LEFT_FRONT 17

#define DIR_RIGHT_BACK 18
#define STEP_RIGHT_BACK 19

#define DIR_LEFT_BACK 20
#define STEP_LEFT_BACK 21
#define MS1 8
#define MS2 9
#define MS3 10
#define RESET 11
#define SLEEP 12

A4988 stepperRF(200, DIR_RIGHT_FRONT, STEP_RIGHT_FRONT);
A4988 stepperLF(200, DIR_LEFT_FRONT,  STEP_LEFT_FRONT);
A4988 stepperRB(200, DIR_RIGHT_BACK,  STEP_RIGHT_BACK);
A4988 stepperLB(200, DIR_LEFT_BACK,   STEP_LEFT_BACK);

char in;
int steps = 180;

char ownRead(){
  if (Serial.available()) return Serial.read();
  return '0';
}


int ownParseInt(){
  if (Serial.available()) return Serial.parseInt();
  return 0;
}

// Returns true when all Motors stopped.
bool allMotorsStopped(){
  int statesum = 0;
  statesum += stepperRF.getCurrentState();
  statesum += stepperLF.getCurrentState();
  statesum += stepperRB.getCurrentState();
  statesum += stepperLB.getCurrentState();
  return statesum == 0 ? true:false;
}

void setup() {

  Serial.begin(115200);
  
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(SLEEP, OUTPUT);
  
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, LOW);
  digitalWrite(RESET, HIGH);
  digitalWrite(SLEEP, HIGH);

  stepperRF.setSpeedProfile(stepperRF.LINEAR_SPEED, 1000, 1000);
  stepperLF.setSpeedProfile(stepperLF.LINEAR_SPEED, 1000, 1000);
  stepperRB.setSpeedProfile(stepperRB.LINEAR_SPEED, 1000, 1000);
  stepperLB.setSpeedProfile(stepperLB.LINEAR_SPEED, 1000, 1000);
  

  stepperRF.begin(60, 8);
  stepperLF.begin(60, 8);
  stepperRB.begin(60, 8);
  stepperLB.begin(60, 8);
  //stepperRF.startRotate(steps   *1);
  //stepperLF.startRotate(-steps  *1);
  //stepperRB.startRotate(steps   *1);
  //stepperLB.startRotate(-steps  *1);
}


void loop() {
  // put your main code here, to run repeatedly:
  /*stepperRF.rotate(360);
  stepperLF.rotate(360);
  stepperRB.rotate(360);
  stepperLB.rotate(360);*/

  in = ownRead();
  if(in != '0'){
    steps = Serial.parseInt();
    while(Serial.available()){
      Serial.read();
    }
    Serial.print("Arduino: command received: ");
    Serial.print(in);
    Serial.print(", Steps: ");
    Serial.println(steps);
  
    switch (in){
      case 'w':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *1);
        stepperLF.startRotate(-steps  *1);
        stepperRB.startRotate(steps   *1);
        stepperLB.startRotate(-steps  *1);
        break;
        
      case 's':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *-1);
        stepperLF.startRotate(-steps  *-1);
        stepperRB.startRotate(steps   *-1);
        stepperLB.startRotate(-steps  *-1);
        break;
        
      case 'a':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *1);
        stepperLF.startRotate(-steps  *-1);
        stepperRB.startRotate(steps   *-1);
        stepperLB.startRotate(-steps  *1);
        break;
  
      case 'd':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *-1);
        stepperLF.startRotate(-steps  *1);
        stepperRB.startRotate(steps   *1);
        stepperLB.startRotate(-steps  *-1);
        break;
        
      case 'q':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *1);
        stepperLF.startRotate(-steps  *0);
        stepperRB.startRotate(steps   *0);
        stepperLB.startRotate(-steps  *1);
        break;
        
      case 'e':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *0);
        stepperLF.startRotate(-steps  *1);
        stepperRB.startRotate(steps   *1);
        stepperLB.startRotate(-steps  *0);
        break;
        
      case 'y':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *0);
        stepperLF.startRotate(-steps  *-1);
        stepperRB.startRotate(steps   *-1);
        stepperLB.startRotate(-steps  *0);
        break;
  
      case 'c':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *-1);
        stepperLF.startRotate(-steps  *0);
        stepperRB.startRotate(steps   *0);
        stepperLB.startRotate(-steps  *-1);
        break;

      case 'r':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *1);
        stepperLF.startRotate(-steps  *-1);
        stepperRB.startRotate(steps   *1);
        stepperLB.startRotate(-steps  *-1);
        break;
  
      case 't':
        digitalWrite(SLEEP, HIGH);
        delay(1); // wait until driver up again
        stepperRF.startRotate(steps   *-1);
        stepperLF.startRotate(-steps  *1);
        stepperRB.startRotate(steps   *-1);
        stepperLB.startRotate(-steps  *1);
        break;

      case 'x':
        if(!allMotorsStopped()){
          digitalWrite(SLEEP, HIGH);
          delay(1); // wait until driver up again
          stepperRF.startBrake();
          stepperLF.startBrake();
          stepperRB.startBrake();
          stepperLB.startBrake();
        }
        break;
     
     default:
        break;
    }
  }
  unsigned wait_time_micros = 0;
  wait_time_micros += stepperRF.nextAction();
  wait_time_micros += stepperLF.nextAction();
  wait_time_micros += stepperRB.nextAction();
  wait_time_micros += stepperLB.nextAction();

  if(wait_time_micros <= 0){
    digitalWrite(SLEEP, LOW);
  }

}
