/*
  TinyServo.cpp - Library for controlling a servo from an ATtiny
  Created by Techniccontroller, May 1, 2019.
  Released into the public domain.
*/

#include "Arduino.h"
#include "TinyServo.h"

TinyServo::TinyServo(int pin)
{
	pinMode(pin, OUTPUT);
	_pin = pin;
	_angle = 90;
	recalcMB();
}

void TinyServo::write(int angle)
{
	_angle = constrain(angle, Servo_minW, Servo_maxW); // limit turning angle
}

void TinyServo::recalcMB()
{
	m = ((Servo_maxpulse - Servo_minpulse)*1.0/(Servo_maxW - Servo_minW)*1.0);
	b = Servo_minpulse - m * Servo_minW;
}

void TinyServo::refresh()
{
	if(active == 1){
		static unsigned long lastRefresh;
		if (millis()-lastRefresh < pulse_distance) return; // noch keine 20 ms vergangen
		lastRefresh = millis(); // Zeit des erfolgten Refresh merken

		int pulselength = _angle * m + b;
		
		noInterrupts();
		digitalWrite(_pin,HIGH);
		delayMicroseconds(pulselength);
		digitalWrite(_pin,LOW);
		interrupts();
	}
	
}

void TinyServo::setMinAngle(int angle){
	Servo_minW = angle;
	recalcMB();
}

void TinyServo::setMaxAngle(int angle){
	Servo_maxW = angle;
	recalcMB();
}

void TinyServo::setMinPulse(int pulse){
	Servo_minpulse = pulse;
	recalcMB();
}

void TinyServo::setMaxPulse(int pulse){
	Servo_maxpulse = pulse;
	recalcMB();
}

void TinyServo::setPulseDistance(int pulse_distance){
	pulse_distance = pulse_distance;
}

void TinyServo::setActive(int value){
	active = value;
}