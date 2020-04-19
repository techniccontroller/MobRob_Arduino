/**
 * Created by techniccontroller 2019
 * 
 * Last modified by techniccontroller 19.04.2020
 */

// include necessary libraries
#include "A4988.h"                // Library for A4988 stepper motor driver
#include "MultiDriverX4.h"
#include <ros.h>
#include <geometry_msgs/Twist.h>
#include <myrobot_model/Pose.h>
#include <roscpp/Empty.h>

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

#define DEBUG 0

#define PI 3.14159265359

ros::NodeHandle  nh;

float g_vx = 0;
float g_vy = 0;
float g_vth = 0;
float tmpX=0, tmpY=0, tmpTheta=0;
float poseX=0, poseY=0, poseTheta=0;

A4988 stepperRF(200, DIR_RIGHT_FRONT, STEP_RIGHT_FRONT);
A4988 stepperLF(200, DIR_LEFT_FRONT,  STEP_LEFT_FRONT);
A4988 stepperRB(200, DIR_RIGHT_BACK,  STEP_RIGHT_BACK);
A4988 stepperLB(200, DIR_LEFT_BACK,   STEP_LEFT_BACK);

MultiDriverX4 controller(stepperRF, stepperLF, stepperRB, stepperLB);

void messageCb( const geometry_msgs::Twist& vel_msg){
  
  
  float vx = vel_msg.linear.x;
  float vy = vel_msg.linear.y;
  float omega = vel_msg.angular.z;
  if(abs(vx) + abs(vy) + abs(omega) < 0.01){
    stopMotors();
  }
  else{
    vel(vx, vy, omega);
    poseX += tmpX * cos(poseTheta + tmpTheta) - tmpY * sin(poseTheta + tmpTheta);
    poseY += tmpX * sin(poseTheta + tmpTheta) + tmpY * cos(poseTheta + tmpTheta);
    poseTheta +=tmpTheta;
  }

  g_vx = vx;
  g_vy = vy;
  g_vth = omega;
}

ros::Subscriber<geometry_msgs::Twist> vel_sub("/cmd_vel", messageCb );

myrobot_model::Pose pose_msg;
ros::Publisher pose_pub("pose", &pose_msg);

void callback(const roscpp::EmptyRequest & req, roscpp::EmptyResponse & res){
  String logmsg = "current-steps: ";
  logmsg += stepperRF.calcRotationForSteps(stepperRF.getStepsCompleted());
  char logmsg_array[logmsg.length()+1];
  logmsg.toCharArray(logmsg_array, logmsg.length()+1);
  nh.loginfo(logmsg_array);
  
  tmpX=0;
  tmpY=0;
  tmpTheta=0;
  poseX=0;
  poseY=0;
  poseTheta=0;
  stepperRF.resetStepsCompleted();
  stepperLF.resetStepsCompleted();
  stepperRB.resetStepsCompleted();
  stepperLB.resetStepsCompleted();
  nh.loginfo("reset pose...");
}

ros::ServiceServer<roscpp::EmptyRequest, roscpp::EmptyResponse> server("/resetPose",&callback);



char in;
int steps = 180;
long infinityMoveSetTime = 0;

char inputbuffer[30] = {0};

int rpmRF=60, rpmLF=60, rpmRB=60, rpmLB=60;
long degRF=0, degLF=0, degRB=0, degLB=0;
long rmstpRF=0, rmstpLF=0, rmstpRB=0, rmstpLB=0;

float radius = 0.06/2; // 3cm
float tanAlpha = tan(0.7853981);  //tan(45°)
float L1 = 0.215/2;
float L2 = 0.29/2;


long lastPubTime = millis();

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

  controller.setSpeedProfile(stepperLB.LINEAR_SPEED, 1000, 1000);
  
  stepperRF.begin(60, 8);
  stepperLF.begin(60, 8);
  stepperRB.begin(60, 8);
  stepperLB.begin(60, 8);

  nh.initNode();
  nh.advertise(pose_pub);
  nh.subscribe(vel_sub);
  nh.advertiseService(server);
}

void vel(float vx, float vy, float omega){
  
  rpmLF = 1/radius * (vx - (1/tanAlpha)*vy - ((L1*tanAlpha + L2)/tanAlpha)*omega) / (2*PI) * 60;
  rpmRF = 1/radius * (vx + (1/tanAlpha)*vy + ((L1*tanAlpha + L2)/tanAlpha)*omega) / (2*PI) * 60;
  rpmLB = 1/radius * (vx + (1/tanAlpha)*vy - ((L1*tanAlpha + L2)/tanAlpha)*omega) / (2*PI) * 60;
  rpmRB = 1/radius * (vx - (1/tanAlpha)*vy + ((L1*tanAlpha + L2)/tanAlpha)*omega) / (2*PI) * 60;

  String logmsg = "rpms: ";
  logmsg += rpmLF;
  logmsg += ", ";
  logmsg += rpmRF;
  logmsg += ", ";
  logmsg += rpmLB;
  logmsg += ", ";
  logmsg += rpmRB;
  char logmsg_array[logmsg.length()+1];
  logmsg.toCharArray(logmsg_array, logmsg.length()+1);
  nh.loginfo(logmsg_array);
  
  int rpmMax = max(max(max(abs(rpmLF), abs(rpmRF)), abs(rpmLB)), abs(rpmRB));
  if(rpmMax > 80){
    float scalingFactor = 80.0/rpmMax;
    rpmLF *= scalingFactor;
    rpmRF *= scalingFactor;
    rpmLB *= scalingFactor;
    rpmRB *= scalingFactor;
    logmsg = "Too high velocity! -> slow down by ";
    logmsg += scalingFactor;
    char logmsg_array2[logmsg.length()+1];
    logmsg.toCharArray(logmsg_array2, logmsg.length()+1);
    nh.loginfo(logmsg_array2);
  }

  
    
  degLF = rpmLF*360 * -1;
  degRF = rpmRF*360;
  degLB = rpmLB*360 * -1;
  degRB = rpmRB*360;
  
  
  infinityMoveSetTime = millis();
  
  if(controller.isRunning()){
    controller.setSpeedProfile(stepperLB.CONSTANT_SPEED, 1000, 1000);
  }else{
    controller.setSpeedProfile(stepperLB.LINEAR_SPEED, 1000, 1000);
  }

  updateMotors();

  
}

void stopMotors(){
  controller.startBrake();
  infinityMoveSetTime = 0;
  
  rmstpRF=stepperRF.getStepsRemaining();
  rmstpLF=stepperLF.getStepsRemaining(); 
  rmstpRB=stepperRB.getStepsRemaining();
  rmstpLB=stepperLB.getStepsRemaining();
}

void updateMotors(){
  if(digitalRead(SLEEP) == LOW){
    digitalWrite(SLEEP, HIGH);
    delay(1); // wait until driver up again
  }
  
  stepperRF.setRPM(abs(rpmRF));
  stepperLF.setRPM(abs(rpmLF));
  stepperRB.setRPM(abs(rpmRB));
  stepperLB.setRPM(abs(rpmLB));
  controller.stop();
  controller.startRotate(degRF, degLF, degRB, degLB);
  rmstpRF=stepperRF.getStepsRemaining();
  rmstpLF=stepperLF.getStepsRemaining(); 
  rmstpRB=stepperRB.getStepsRemaining();
  rmstpLB=stepperLB.getStepsRemaining();
}

void updatePose(){
  int dirRF = stepperRF.getDirection();
  int dirLF = stepperLF.getDirection()*-1;
  int dirRB = stepperRB.getDirection();
  int dirLB = stepperLB.getDirection()*-1;
  
  float diffRF = dirRF * stepperRF.calcRotationForSteps(stepperRF.getStepsCompleted()) * 2 * PI / 360;
  float diffLF = dirLF * stepperLF.calcRotationForSteps(stepperLF.getStepsCompleted()) * 2 * PI / 360;
  float diffRB = dirRB * stepperRB.calcRotationForSteps(stepperRB.getStepsCompleted()) * 2 * PI / 360;
  float diffLB = dirLB * stepperLB.calcRotationForSteps(stepperLB.getStepsCompleted()) * 2 * PI / 360;

  tmpX = radius/4.0 * (diffLF + diffRF + diffLB + diffRB);
  tmpY = radius/4.0 * (-diffLF + diffRF + diffLB - diffRB);
  tmpTheta = radius/4.0 * (-1*diffLF/(L1+L2) + diffRF/(L1+L2) - diffLB/(L1+L2) + diffRB/(L1+L2));

  /*String logmsg = "diffs/tmpX/poseX: ";
  logmsg += diffRF;
  logmsg += ", ";
  logmsg += diffLF;
  logmsg += ", ";
  logmsg += diffRB;
  logmsg += ", ";
  logmsg += diffLB;
  logmsg += ", ";
  logmsg += tmpX;
  logmsg += ", ";
  logmsg += poseX;
  char logmsg_array[logmsg.length()+1];
  logmsg.toCharArray(logmsg_array, logmsg.length()+1);
  nh.loginfo(logmsg_array);*/
  
  pose_msg.x = poseX + tmpX * cos(poseTheta + tmpTheta) - tmpY * sin(poseTheta + tmpTheta);
  pose_msg.y = poseY + tmpX * sin(poseTheta + tmpTheta) + tmpY * cos(poseTheta + tmpTheta);
  pose_msg.theta = poseTheta + tmpTheta;

  pose_msg.vx = g_vx;
  pose_msg.vy = g_vy;
  pose_msg.vth = g_vth;
  
  pose_pub.publish( &pose_msg );
}

void loop() {

  if(controller.nextAction() <= 0){
    digitalWrite(SLEEP, LOW);
  }

  if(infinityMoveSetTime != 0 && (millis()-infinityMoveSetTime > 50000)){
    controller.startRotate(degRF, degLF, degRB, degLB);
    infinityMoveSetTime = millis();
  }

  if(millis() - lastPubTime > 100){
     updatePose();
     lastPubTime = millis();
  }

  nh.spinOnce();
}
