// Read Line command

#include "A4988.h"
#include "MultiDriverX4.h"

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

#define DEBUG 1

A4988 stepperRF(200, DIR_RIGHT_FRONT, STEP_RIGHT_FRONT);
A4988 stepperLF(200, DIR_LEFT_FRONT,  STEP_LEFT_FRONT);
A4988 stepperRB(200, DIR_RIGHT_BACK,  STEP_RIGHT_BACK);
A4988 stepperLB(200, DIR_LEFT_BACK,   STEP_LEFT_BACK);

MultiDriverX4 controller(stepperRF, stepperLF, stepperRB, stepperLB);

char in;
int steps = 180;

char inputbuffer[30] = {0};

char ownRead(){
  if (Serial.available()) return Serial.read();
  return '0';
}

int ownReadLine(){
  int i = 0;
  for(int i = 0; i < 30; i++){
    inputbuffer[i] = 0;
  }
  delay(2);
  while (Serial.available()){
    char in = Serial.read();
    if(in == '\n'){
      inputbuffer[i] = in;
      inputbuffer[i+1] = '\0';
      break;
    }
    inputbuffer[i] = in;
    i++; 
  }
  // return number of character read
  return i;
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
  Serial.println("Start program...");
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
}

void getParameters(char* str, int* params){
  char* param = strtok(str, ",)");
  int i = 0;
  while(param != 0){
    params[i] = atoi(param);
    param = strtok(0, ",)");
    i++;
  }
}


void loop() {
  // Read serial data input when available
  int inputlen = 0;
  if(Serial.available()){
    inputlen = ownReadLine();
  } 
  if(inputlen > 0){
    Serial.print("command received: ");
    Serial.print(inputbuffer);
    
    char* startParam = strchr(inputbuffer, '(');
    // Split the string
    *startParam = '\0';
    startParam++;
    
    int rpmRF=60, rpmLF=60, rpmRB=60, rpmLB=60;
    int degRF=0, degLF=0, degRB=0, degLB=0;
    
    if(strcmp(inputbuffer, "move") == 0){    
      // 'move(distance, speed, direction)' command received

      // split string in int-array;
      int params[3] = {0};
      getParameters(startParam, params);
      int robotdistance = params[0];
      int robotspeed = params[1];
      int robotdirection = params[2];
      
      float angle45 = (robotdirection + 45) * PI / 180.0;
       
      rpmRF = (robotspeed * cos(angle45));
      rpmLF = (robotspeed * sin(angle45));
      rpmRB = (robotspeed * sin(angle45));
      rpmLB = (robotspeed * cos(angle45));
      
      degRF = (robotspeed > 0 ? robotdistance * rpmRF/(robotspeed*1.0) : 0);
      degLF = ((robotspeed > 0 ? robotdistance * rpmLF/(robotspeed*1.0) : 0)) * -1;
      degRB = (robotspeed > 0 ? robotdistance * rpmRB/(robotspeed*1.0) : 0);
      degLB = ((robotspeed > 0 ? robotdistance * rpmLB/(robotspeed*1.0) : 0)) * -1;
    }
    else if(strcmp(inputbuffer, "turn") == 0){    
      // 'turn(angle, speed, radius)' command received
      
      // split string in int-array;
      int params[3] = {0};
      getParameters(startParam, params);
      // Apply correctionfactor
      int robotangle = params[0] * 4.8;
      int robotspeed = params[1];
      int robotradius = params[2]; // in cm
      
      if(robotangle < 0) robotradius = robotradius *-1;
      
      int rpmRF, rpmLF, rpmRB, rpmLB;
      int degRF, degLF, degRB, degLB;
       
      rpmRF = robotspeed * (robotradius-11) / 10;
      rpmLF = robotspeed * (robotradius+11) / 10;
      rpmRB = robotspeed * (robotradius-11) / 10;
      rpmLB = robotspeed * (robotradius+11) / 10;

      int umfangWheel = 2*PI*6; // in cm
      
      degRF = robotangle * 2 * PI * (robotradius-11) / umfangWheel;
      degLF = robotangle * 2 * PI * (robotradius+11) / umfangWheel * -1;
      degRB = robotangle * 2 * PI * (robotradius-11) / umfangWheel;
      degLB = robotangle * 2 * PI * (robotradius+11) / umfangWheel * -1;
    }

#if DEBUG
    Serial.print("rpmRF: ");
    Serial.print(rpmRF);
    Serial.print(", rpmLF: ");
    Serial.print(rpmLF);
    Serial.print(", rpmRB: ");
    Serial.print(rpmRB);
    Serial.print(", rpmLB: ");
    Serial.println(rpmLB);

    Serial.print("degRF: ");
    Serial.print(degRF);
    Serial.print(", degLF: ");
    Serial.print(degLF);
    Serial.print(", degRB: ");
    Serial.print(degRB);
    Serial.print(", degLB: ");
    Serial.println(degLB);
#endif

    digitalWrite(SLEEP, HIGH);
    delay(1); // wait until driver up again
    stepperRF.setRPM(abs(rpmRF));
    stepperLF.setRPM(abs(rpmLF));
    stepperRB.setRPM(abs(rpmRB));
    stepperLB.setRPM(abs(rpmLB));
    
    controller.startRotate(degRF, degLF, degRB, degLB);
  }

  if(controller.nextAction() <= 0){
    digitalWrite(SLEEP, LOW);
  }
}
