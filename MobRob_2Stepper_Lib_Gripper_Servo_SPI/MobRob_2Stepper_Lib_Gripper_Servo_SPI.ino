/*
 * Created by Techniccontroller 2019
 */
#include <Arduino.h>
#include "A4988.h"
#include "TinyServo.h"
//#include "MultiDriver.h"

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

#define SERVO 9

#define MISO 5
#define MOSI 6
#define SCLK 4

#define DEBUG 0

A4988 stepperGRIP(200, DIR_GRIP, STEP_GRIP);
A4988 stepperVERT(200, DIR_VERTICAL,  STEP_VERTICAL);
TinyServo servo(SERVO);
long targetPositionVERT = 0;
long targetPositionGRIP = 0;
long currentPositionVERT = 0;
long currentPositionGRIP = 0;

//MultiDriver controller(stepperGRIP, stepperVERT);
bool letStepperActive = false;

// variables for SPI Communication
char inputbuffer[50];
volatile byte pos = 0;
volatile boolean process_it = false;
volatile int cmdcounter = 33;

// variables for servo
long lasttime = millis();

// move gripper gripper to initial position
void initGripper(){
  digitalWrite(SLEEP, HIGH);
  stepperGRIP.startRotate(-360*2);
  unsigned wait_time_micros = stepperGRIP.nextAction();
  while(wait_time_micros > 0 && digitalRead(END_GRIP) == HIGH){
    wait_time_micros = stepperGRIP.nextAction();
  }
  stepperGRIP.stop();
  currentPositionGRIP = 0;
}

// move vetical motor to initial position
void initVertical(){
  digitalWrite(SLEEP, HIGH);
  stepperVERT.startRotate(-360*3);
  unsigned wait_time_micros = stepperVERT.nextAction();
  while(wait_time_micros > 0 && digitalRead(END_VERT) == LOW){
    wait_time_micros = stepperVERT.nextAction();
  }
  stepperVERT.stop();
  currentPositionVERT = 0;
}

// extract paramerts from comma separated string
void getParameters(char* str, int* params){
  char* param = strtok(str, ",)");
  int i = 0;
  while(param != 0){
    params[i] = atoi(param);
    param = strtok(0, ",)");
    i++;
  }
}

// update absolute position of vertical motor
void updatePositionVERT(){
  if(stepperVERT.getDirection() == HIGH){
    currentPositionVERT = targetPositionVERT - stepperVERT.getRemainingSteps();
  }else{
    currentPositionVERT = targetPositionVERT + stepperVERT.getRemainingSteps();
  }
}

// update absolute position of gripper motor
void updatePositionGRIP(){
  if(stepperGRIP.getDirection() == HIGH){
    currentPositionGRIP = targetPositionGRIP - stepperGRIP.getRemainingSteps();
  }else{
    currentPositionGRIP = targetPositionGRIP + stepperGRIP.getRemainingSteps();
  }
}

// setup function
void setup() {
  pinMode(SLEEP, OUTPUT);
  pinMode(END_GRIP, INPUT_PULLUP);
  pinMode(END_VERT, INPUT_PULLUP);
  digitalWrite(SLEEP, HIGH);
  stepperGRIP.begin(RPM, MICROSTEPS);
  stepperVERT.begin(RPM, MICROSTEPS);

  //controller.setSpeedProfile(stepperGRIP.LINEAR_SPEED, 500, 1000);
  stepperVERT.setSpeedProfile(stepperVERT.LINEAR_SPEED, 500, 1000);
  stepperGRIP.setSpeedProfile(stepperGRIP.LINEAR_SPEED, 500, 1000);
  
  //initVertical();
  //initGripper();

  // set camera to initial position
  //servo.write(50);
  servo.setActive(0);

  // Set Pin for servo as output
  pinMode(SERVO, OUTPUT);

 
  // get ready for an interrupt 
  pos = 0;          // buffer empty
  process_it = false;

  // turn on slave mode
  // - enable overflow interrupt
  // - set Three-wire Mode
  // - set external clock source
  USICR |= (1<<USIOIE)|(1<<USIWM0)|(1<<USICS1);
  // clear interrupt
  USISR = (1<<USIOIF);
  
  // set pin modes
  pinMode(MISO, OUTPUT);
  pinMode(MOSI, INPUT);
  pinMode(SCLK, INPUT);

  // enable interupts
  sei();
  USIDR = 42;
}

// SPI interrupt routine
ISR (USI_OVF_vect){
  char c = USIDR;

  // add to buffer if room
  if (pos < (50 - 1))
    inputbuffer[pos++] = c;
  
  if (c == 10)
    process_it = true;

  // convert lowercase letters in uppercase
  if (c >=97){
    c -= 32;  
  }
  USIDR = c;
  USISR = (1<<USIOIF);
}

void loop() {
  
  if (process_it){    
    inputbuffer[pos] = '\0';  
    pos = 0;
    cmdcounter++;
    // get the start of parameters in the command
    char* startParam = strchr(inputbuffer, '(');
    // get the start of parameters in the command
    char* startFunction = strchr(inputbuffer, '_');
    if(startParam != NULL && startFunction != NULL){
      // Split the string
      *startParam = '\0';
      *startFunction = '\0';
      startParam++;
      startFunction++;
  
      // ##################################################################################################### //
      //                                      SERVO
      // ##################################################################################################### //
      if(strcmp(inputbuffer, "servo") == 0){
        // command for servo received
        if(strcmp(startFunction, "active") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int active = params[0];
          servo.setActive(active);
        }
        else if(strcmp(startFunction, "write") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int angle = params[0];
          servo.setActive(1);
          servo.write(angle);
        }
      }
      // ##################################################################################################### //
      //                                      VERTICAL
      // ##################################################################################################### //
      else if(strcmp(inputbuffer, "vertical") == 0){
        // command for vertical motor received
        if(strcmp(startFunction, "init") == 0){
          // split string in int-array;
          initVertical();
        }
        else if(strcmp(startFunction, "setSpeed") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int rpmVERT = params[0];
          stepperVERT.setRPM(abs(rpmVERT));
        }
        else if(strcmp(startFunction, "moveAbs") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int distAbs = params[0];
          targetPositionVERT = distAbs;
          stepperVERT.stop();
          stepperVERT.startMove(targetPositionVERT - currentPositionVERT);
        }
        else if(strcmp(startFunction, "moveRel") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int distRel = params[0];
          targetPositionVERT += distRel;
          stepperVERT.stop();
          stepperVERT.startMove(targetPositionVERT - currentPositionVERT);
          //stepperVERT.startRotate(distRel);
        }
        else if(strcmp(startFunction, "stop") == 0){
          stepperVERT.stop();
          targetPositionVERT = currentPositionVERT;
        }
      }
      // ##################################################################################################### //
      //                                      GRIPPER
      // ##################################################################################################### //
      else if(strcmp(inputbuffer, "gripper") == 0){
        // command for gripper motor received
        if(strcmp(startFunction, "init") == 0){
          // split string in int-array;
          initGripper();
        }
        else if(strcmp(startFunction, "setSpeed") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int rpmGRIP = params[0];
          stepperGRIP.setRPM(abs(rpmGRIP));
        }
        else if(strcmp(startFunction, "moveAbs") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int distAbs = params[0];
          targetPositionGRIP = distAbs;
          stepperGRIP.stop();
          stepperGRIP.startMove(targetPositionGRIP - currentPositionGRIP);
        }
        else if(strcmp(startFunction, "moveRel") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int distRel = params[0];
          targetPositionGRIP += distRel;
          stepperGRIP.stop();
          stepperGRIP.startMove(targetPositionGRIP - currentPositionGRIP);
          //stepperGRIP.startRotate(distRel);
        }
        else if(strcmp(startFunction, "stop") == 0){
          stepperGRIP.stop();
          targetPositionGRIP = currentPositionGRIP;
        }
      }
      // ##################################################################################################### //
      //                                      MOTORS
      // ##################################################################################################### //
      else if(strcmp(inputbuffer, "stepper") == 0){
        // command for stepper motors received
        if(strcmp(startFunction, "active") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int active = params[0];
          letStepperActive = active;
        }
        else if(strcmp(startFunction, "stop") == 0){
          stepperGRIP.stop();
          stepperVERT.stop();
          letStepperActive == 0;
          targetPositionGRIP = currentPositionGRIP;
          targetPositionVERT = currentPositionVERT;
        }
      }
      // ##################################################################################################### //
      
    }
    process_it = false;
  }

  int waittimeVERT = stepperVERT.nextAction();
  int waittimeGRIP = stepperGRIP.nextAction();
  if(waittimeVERT <= 0 && waittimeGRIP <= 0 && letStepperActive == false){
    digitalWrite(SLEEP, LOW);
  }
  else{
    digitalWrite(SLEEP, HIGH);
  }
  
  updatePositionGRIP();
  updatePositionVERT();

  if(pos == 0 && millis()-lasttime > 20){
    //servo.refresh();
    lasttime = millis();
  }  
}
