/*
  TinyServo.h - Library for controlling a servo from an ATtiny
  Created by Techniccontroller, May 1, 2019.
  Released into the public domain.
*/

#ifndef TinyServo_h
#define TinyServo_h

#include "Arduino.h"

class TinyServo
{
public:
	TinyServo(int pin);
	void write(int angle);
	void refresh();
	void setMinAngle(int angle);
	void setMaxAngle(int angle);
	void setMinPulse(int pulse);
	void setMaxPulse(int pulse);
	void setPulseDistance(int pulse_distance);
	void setActive(int active);
	int getAngle();

private:
	void recalcMB();
	int _pin;
	int _angle;
	int active = 0;
	int pulse_distance = 20;   // pause between every pulse
	int Servo_minpulse = 700;  // min pulselength -> 0deg
	int Servo_maxpulse = 2750; // max pulselength -> 180deg
	int Servo_minW = 0;		   // limits for turning angle
	int Servo_maxW = 180;
	float m = 1.0;
	int b = 0;
};

#endif