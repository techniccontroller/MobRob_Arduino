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

#define RECEIVE 1
#define SEND 2

#define LIMIT_VERT 4500
#define LIMIT_GRIP 2900

A4988 stepperGRIP(200, DIR_GRIP, STEP_GRIP);
A4988 stepperVERT(200, DIR_VERTICAL,  STEP_VERTICAL);
TinyServo servo(SERVO);
long targetPositionVERT = 0;
long targetPositionGRIP = 0;
long currentPositionVERT = 0;
long currentPositionGRIP = 0;
bool isInitializedVERT = false;
bool isInitializedGRIP = false;

//MultiDriver controller(stepperGRIP, stepperVERT);
bool letStepperActive = false;

// variables for SPI Communication
char inputbuffer[50];
char outputbuffer[50];
volatile byte pos = 0;
volatile byte posOut = 0;
volatile boolean process_it = false;
volatile int cmdcounter = 33;

// variables for servo
long lasttime = millis();

// spi communication state
int spiState = RECEIVE;

bool isEndGripPressed(){
  return digitalRead(END_GRIP) == LOW;
}

bool isEndVertPressed(){
  return digitalRead(END_VERT) == HIGH;
}

// move gripper gripper to initial position
void initGripper(){
  digitalWrite(SLEEP, HIGH);
  stepperGRIP.startRotate(-360*2);
  unsigned wait_time_micros = stepperGRIP.nextAction();
  while(wait_time_micros > 0 && !isEndGripPressed()){
    wait_time_micros = stepperGRIP.nextAction();
  }
  stepperGRIP.stop();
  currentPositionGRIP = 0;
  targetPositionGRIP  = 0;
  isInitializedGRIP = true;
}

// move vetical motor to initial position
void initVertical(){
  digitalWrite(SLEEP, HIGH);
  stepperVERT.startRotate(-360*3);
  unsigned wait_time_micros = stepperVERT.nextAction();
  while(wait_time_micros > 0 && !isEndVertPressed()){
    wait_time_micros = stepperVERT.nextAction();
  }
  stepperVERT.stop();
  currentPositionVERT = 0;
  targetPositionVERT  = 0;
  isInitializedVERT = true;
}

void stopGripper(){
  stepperGRIP.stop();
  targetPositionGRIP = currentPositionGRIP;
}

void stopVertical(){
  stepperVERT.stop();
  targetPositionVERT = currentPositionVERT;
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
  if(stepperVERT.getDirection() == 1){
    currentPositionVERT = targetPositionVERT - stepperVERT.getStepsRemaining();
  }else{
    currentPositionVERT = targetPositionVERT + stepperVERT.getStepsRemaining();
  }
}

// update absolute position of gripper motor
void updatePositionGRIP(){
  if(stepperGRIP.getDirection() == 1){
    currentPositionGRIP = targetPositionGRIP - stepperGRIP.getStepsRemaining();
  }else{
    currentPositionGRIP = targetPositionGRIP + stepperGRIP.getStepsRemaining();
  }
}

// convert mm in absolute steps for vertical motor
int toStepsAbsVERT(int mmPos){
  return mmPos * -40 + 5680;
}

// convert steps in mm for vertical motor
int toMMAbsVERT(int stepsPos){
  return (stepsPos - 5680)/-40;
}

// convert mm in relative steps for vertical motor
int toStepsRelVERT(int mmPos){
  return mmPos * -40;
}

// convert steps in mm for vertical motor
int toMMAbsRelVERT(int stepsPos){
  return stepsPos / -40;
}

// convert mm in steps for gripper
int toStepsGRIP(int mmPos){
  return mmPos * 19;
}

// convert steps in mm for gripper
int toMMGRIP(int stepsPos){
  return stepsPos / 19;
}

void fillOutputBuffer(uint16_t value){
  uint8_t i = 2;           
  uint16_t d = 1000;
  for(; i < 6; i++, d /= 10){
    outputbuffer[i] = ((uint16_t)(value/d))%10 + 48;
  }
  outputbuffer[6] = '\n';
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

  outputbuffer[0] = '0';
  outputbuffer[1] = '0';
  outputbuffer[2] = '0';
  outputbuffer[3] = '0';
  outputbuffer[4] = '0';
  outputbuffer[5] = '0';
  outputbuffer[6] = '\n';
}

// SPI interrupt routine
ISR (USI_OVF_vect){
  char c = USIDR;

  if(spiState == RECEIVE){
    // add to buffer if room
    if (pos < (50 - 1))
      inputbuffer[pos++] = c;
  
    // end of line (\n) detected -> process the command
    if (c == 10)
      process_it = true;
  
    // convert lowercase letters in uppercase
    if (c >=97){
      c -= 32;  
    }
    USIDR = c;
  }
  else if(spiState == SEND){
    if(outputbuffer[posOut] == '\n'){
      spiState = RECEIVE;
    }
    USIDR = outputbuffer[posOut++];
  }
  
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
      if(strcmp(inputbuffer, "sv") == 0){
        // command for servo received
        if(strcmp(startFunction, "ac") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int active = params[0];
          servo.setActive(active);
        }
        else if(strcmp(startFunction, "wr") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int angle = params[0];
          servo.setActive(1);
          servo.write(angle);
        }
        else if(strcmp(startFunction, "rf") == 0){
          servo.refresh();
        }
        else if(strcmp(startFunction, "gp") == 0){
          fillOutputBuffer(servo.getAngle());
          spiState = SEND;
          posOut = 0;
        }
      }
      // ##################################################################################################### //
      //                                      VERTICAL
      // ##################################################################################################### //
      else if(strcmp(inputbuffer, "vt") == 0){
        // command for vertical motor received
        if(strcmp(startFunction, "it") == 0){
          // split string in int-array;
          initVertical();
        }
        else if(strcmp(startFunction, "sp") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int rpmVERT = params[0];
          stepperVERT.setRPM(abs(rpmVERT));
        }
        else if(strcmp(startFunction, "ma") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int distAbs = params[0];
          targetPositionVERT = toStepsAbsVERT(distAbs);
          if(targetPositionVERT < 0)
            targetPositionVERT = 0;
          else if(targetPositionVERT > LIMIT_VERT)
            targetPositionVERT = LIMIT_VERT;
          stepperVERT.stop();
          stepperVERT.startMove(targetPositionVERT - currentPositionVERT);
        }
        else if(strcmp(startFunction, "mr") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int distRel = params[0];
          targetPositionVERT += toStepsRelVERT(distRel);
          if(targetPositionVERT < 0)
            targetPositionVERT = 0;
          else if(targetPositionVERT > LIMIT_VERT)
            targetPositionVERT = LIMIT_VERT;
          stepperVERT.stop();
          stepperVERT.startMove(targetPositionVERT - currentPositionVERT);
          //stepperVERT.startRotate(distRel);
        }
        else if(strcmp(startFunction, "st") == 0){
          stopVertical();
        }
        else if(strcmp(startFunction, "gp") == 0){
          if(isInitializedVERT){
            fillOutputBuffer(toMMAbsVERT(currentPositionVERT));
          }else{
            fillOutputBuffer(9999);
          }
          outputbuffer[6] = '\n';
          spiState = SEND;
          posOut = 0;
        }
      }
      // ##################################################################################################### //
      //                                      GRIPPER
      // ##################################################################################################### //
      else if(strcmp(inputbuffer, "gr") == 0){
        // command for gripper motor received
        if(strcmp(startFunction, "it") == 0){
          initGripper();
        }
        else if(strcmp(startFunction, "sp") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int rpmGRIP = params[0];
          stepperGRIP.setRPM(abs(rpmGRIP));
        }
        else if(strcmp(startFunction, "ma") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int distAbs = params[0];
          targetPositionGRIP = toStepsGRIP(distAbs);
          if(targetPositionGRIP < 0)
            targetPositionGRIP = 0;
          else if(targetPositionGRIP > LIMIT_GRIP)
            targetPositionGRIP = LIMIT_GRIP;
          stepperGRIP.stop();
          stepperGRIP.startMove(targetPositionGRIP - currentPositionGRIP);
        }
        else if(strcmp(startFunction, "mr") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int distRel = params[0];
          targetPositionGRIP += toStepsGRIP(distRel);
          if(targetPositionGRIP < 0)
            targetPositionGRIP = 0;
          else if(targetPositionGRIP > LIMIT_GRIP)
            targetPositionGRIP = LIMIT_GRIP;
          stepperGRIP.stop();
          stepperGRIP.startMove(targetPositionGRIP - currentPositionGRIP);
          //stepperGRIP.startRotate(distRel);
        }
        else if(strcmp(startFunction, "st") == 0){
          stopGripper();
        }
        else if(strcmp(startFunction, "gp") == 0){
          if(isInitializedGRIP){
            fillOutputBuffer(toMMGRIP(currentPositionGRIP));
          }else{
            fillOutputBuffer(9999);
          }
          spiState = SEND;
          posOut = 0;
        }
        else if(strcmp(startFunction, "gg") == 0){
          fillOutputBuffer(isEndGripPressed());
          spiState = SEND;
          posOut = 0;
        }
      }
      // ##################################################################################################### //
      //                                      MOTORS
      // ##################################################################################################### //
      else if(strcmp(inputbuffer, "st") == 0){
        // command for stepper motors received
        if(strcmp(startFunction, "ac") == 0){
          // split string in int-array;
          int params[1] = {0};
          getParameters(startParam, params);
          int active = params[0];
          letStepperActive = active;
        }
        else if(strcmp(startFunction, "st") == 0){
          stopGripper();
          stopVertical();
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

  if(isEndGripPressed() && (stepperGRIP.getDirection() == -1) && waittimeGRIP > 0){
    stopGripper();
  }
  if(isEndVertPressed() && (stepperVERT.getDirection() == -1) && waittimeVERT > 0){
    stopVertical();
  }
  
  updatePositionGRIP();
  updatePositionVERT();
}
