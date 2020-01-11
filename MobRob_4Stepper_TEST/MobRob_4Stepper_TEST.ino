#define DIR_RIGHT_FRONT 0
#define STEP_RIGHT_FRONT 1

#define DIR_LEFT_FRONT 2
#define STEP_LEFT_FRONT 3

#define DIR_RIGHT_BACK 4
#define STEP_RIGHT_BACK 5

#define DIR_LEFT_BACK 6
#define STEP_LEFT_BACK 7
#define MS1 8
#define MS2 9
#define MS3 10

int stepPin = STEP_RIGHT_FRONT; 
int dirPin = DIR_RIGHT_FRONT; 

void setup() {
  // Sets the pins as Outputs
  pinMode(DIR_RIGHT_FRONT,OUTPUT); 
  pinMode(STEP_RIGHT_FRONT,OUTPUT);
  
  pinMode(DIR_LEFT_FRONT,OUTPUT); 
  pinMode(STEP_LEFT_FRONT,OUTPUT);
  
  pinMode(DIR_RIGHT_BACK,OUTPUT); 
  pinMode(STEP_RIGHT_BACK,OUTPUT);
  
  pinMode(DIR_LEFT_BACK,OUTPUT); 
  pinMode(STEP_LEFT_BACK,OUTPUT);

  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);

  digitalWrite(DIR_RIGHT_FRONT, LOW);
  digitalWrite(STEP_RIGHT_FRONT, LOW);
  digitalWrite(DIR_LEFT_FRONT, HIGH);
  digitalWrite(STEP_LEFT_FRONT, LOW);
  digitalWrite(DIR_RIGHT_BACK, LOW);
  digitalWrite(STEP_RIGHT_BACK, LOW);
  digitalWrite(DIR_LEFT_BACK, HIGH);
  digitalWrite(STEP_LEFT_BACK, LOW);

  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  
}
int delaySteps = 2000;

void loop() {
  digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  for(int x = 0; x < 200; x++) {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(5); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(delaySteps); 
  }
  delay(1000); // One second delay
  
  digitalWrite(dirPin,LOW); //Changes the rotations direction
  // Makes 400 pulses for making two full cycle rotation
  for(int x = 0; x < 400; x++) {
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(5);
    digitalWrite(stepPin,LOW);
    delayMicroseconds(delaySteps);
  }
  delay(1000);

  stepPin = (stepPin + 2)%8; 
  dirPin = (dirPin + 2)%8;

}
