/*
 * Multi-motor group driver
 *
 * Copyright (C)2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
#ifndef MULTI_DRIVER_H
#define MULTI_DRIVER_H
#include <Arduino.h>
#include "BasicStepperDriver.h"

#define MAX_MOTORS 4    // a reasonable but arbitrary limit
#define Motor BasicStepperDriver
/*
 * Multi-motor group driver class.
 */
class MultiDriverX4 {
protected:
    /*
     * Configuration
     */
    unsigned short count;
    Motor* const *motors;
    /*
     * Generic initializer, will be called by the others
     */
    MultiDriverX4(const unsigned short count, Motor* const *motors)
    :count(count), motors(motors)
    {};

    /*
     * Movement state
     */
    // ready to start a new move
    bool ready = true;
    // when next state change is due for each motor
    long event_timers[MAX_MOTORS];
    unsigned long next_action_interval = 0;
    unsigned long last_action_end = 0;

public:
    /*
     * Two-motor setup
     */
    MultiDriverX4(Motor& motor1, Motor& motor2)
    :MultiDriverX4(2, new Motor* const[2]{&motor1, &motor2})
    {};
    /*
     * Three-motor setup (X, Y, Z for example)
     */
    MultiDriverX4(Motor& motor1, Motor& motor2, Motor& motor3)
    :MultiDriverX4(3, new Motor* const[3]{&motor1, &motor2, &motor3})
    {};
	
	/*
     * Four-motor setup (mecanum robot for example)
     */
    MultiDriverX4(Motor& motor1, Motor& motor2, Motor& motor3, Motor& motor4)
    :MultiDriverX4(4, new Motor* const[4]{&motor1, &motor2, &motor3, &motor4})
    {};
    
	
	unsigned short getCount(void){
        return count;
    }
    Motor& getMotor(short index){
        return *motors[index];
    }
    /*
     * Move the motors a given number of steps.
     * positive to move forward, negative to reverse
     */
    void move(long steps1, long steps2, long steps3=0, long steps4=0);
    void rotate(int deg1, int deg2, int deg3=0, int deg4=0){
        rotate((long)deg1, (long)deg2, (long)deg3, (long)deg4);
    };
    void rotate(long deg1, long deg2, long deg3=0, long deg4=0);
    void rotate(double deg1, double deg2, double deg3=0, double deg4=0);

    /*
     * Motor movement with external control of timing
     */
    virtual void startMove(long steps1, long steps2, long steps3=0, long steps4=0);
    void startRotate(int deg1, int deg2, int deg3=0, int deg4=0){
        startRotate((long)deg1, (long)deg2, (long)deg3, (long)deg4);
    };
    void startRotate(long deg1, long deg2, long deg3=0, long deg4=0);
    void startRotate(double deg1, double deg2, double deg3=0, double deg4=0);
    /*
     * Toggle step and return time until next change is needed (micros)
     */
    virtual long nextAction(void);
    /*
     * Optionally, call this to begin braking to stop early
     */
    void startBrake(void);
	/*
     * Optionally, call this to begin braking to stop early
     */
    void stop(void);
    /*
     * State querying
     */
    bool isRunning(void);
     
    /*
     * Set the same microstepping level on all motors
     */
    void setMicrostep(unsigned microsteps);
    /*
     * Turn all motors on or off
     */
    void enable(void);
    void disable(void);
	
	void setSpeedProfile(BasicStepperDriver::Mode mode, short accel, short decel);
};
#endif // MULTI_DRIVER_H
