/*
 * Multi-motor group driver
 *
 * Copyright (C)2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 */
#include "MultiDriverX4.h"

#define FOREACH_MOTOR(action) for (short i=count-1; i >= 0; i--){action;}

/*
 * Initialize motor parameters
 */
void MultiDriverX4::startMove(long steps1, long steps2, long steps3, long steps4){
    long steps[4] = {steps1, steps2, steps3, steps4};
    /*
     * Initialize state for all active motors
     */
    FOREACH_MOTOR(
        if (steps[i]){
            motors[i]->startMove(steps[i]);
            event_timers[i] = 0;
        } else {
            event_timers[i] = -1;
        }
    );
    ready = false;
    last_action_end = 0;
}
/*
 * Trigger next step action
 */
long MultiDriverX4::nextAction(void){
    Motor::delayMicros(next_action_interval, last_action_end);
    next_action_interval = 0;
    
    // Trigger all the motors that need it (event timer = 0)
    FOREACH_MOTOR(
        if (event_timers[i] == 0){
            event_timers[i] = motors[i]->nextAction();
        }
    );
    // Find the time when the next pulse needs to fire
    // this is the smallest non-zero timer value from all active motors
    ready = true;
    FOREACH_MOTOR(
        if (event_timers[i] > 0){
            ready = false;
            if (event_timers[i] < next_action_interval || next_action_interval == 0){
                next_action_interval = event_timers[i];
            }
        }
    );
    // Reduce all event timers by the current left time so 0 marks next
    FOREACH_MOTOR(
        if (event_timers[i] > 0){
            event_timers[i] -= next_action_interval;
        }
    );
    last_action_end = 0;
    return next_action_interval;
}
/*
 * Optionally, call this to begin braking to stop early
 */
void MultiDriverX4::startBrake(void){
    FOREACH_MOTOR(
        if (event_timers[i] >= 0){
            motors[i]->startBrake();
        }
    )
}

/*
 * Optionally, call this to begin braking to stop early
 */
void MultiDriverX4::stop(void){
    FOREACH_MOTOR(
        if (event_timers[i] >= 0){
            motors[i]->stop();
        }
    )
}

/*
 * State querying
 */
bool MultiDriverX4::isRunning(void){
    bool running = false;
    FOREACH_MOTOR(
        if (motors[i]->getCurrentState() != Motor::STOPPED){
            running = true;
            break;
        }
    )
    return running;
}

/*
 * Move each motor the requested number of steps, in parallel
 * positive to move forward, negative to reverse, 0 to remain still
 */
void MultiDriverX4::move(long steps1, long steps2, long steps3, long steps4){
    unsigned long next_event;
    startMove(steps1, steps2, steps3, steps4);
    while (!ready){
        next_event = nextAction();
    }
}

#define CALC_STEPS(i, deg) ((motors[i] && deg) ? motors[i]->calcStepsForRotation(deg) : 0)
void MultiDriverX4::rotate(long deg1, long deg2, long deg3, long deg4){
    move(CALC_STEPS(0, deg1), CALC_STEPS(1, deg2), CALC_STEPS(2, deg3), CALC_STEPS(3, deg4));
}

void MultiDriverX4::rotate(double deg1, double deg2, double deg3, double deg4){
    move(CALC_STEPS(0, deg1), CALC_STEPS(1, deg2), CALC_STEPS(2, deg3), CALC_STEPS(3, deg4));
}

void MultiDriverX4::startRotate(long deg1, long deg2, long deg3, long deg4){
    startMove(CALC_STEPS(0, deg1), CALC_STEPS(1, deg2), CALC_STEPS(2, deg3), CALC_STEPS(3, deg4));
}

void MultiDriverX4::startRotate(double deg1, double deg2, double deg3, double deg4){
    startMove(CALC_STEPS(0, deg1), CALC_STEPS(1, deg2), CALC_STEPS(2, deg3), CALC_STEPS(3, deg4));
}

void MultiDriverX4::setMicrostep(unsigned microsteps){
    FOREACH_MOTOR(motors[i]->setMicrostep(microsteps));
}

void MultiDriverX4::enable(void){
    FOREACH_MOTOR(motors[i]->enable());
}
void MultiDriverX4::disable(void){
    FOREACH_MOTOR(motors[i]->disable());
}

void MultiDriverX4::setSpeedProfile(BasicStepperDriver::Mode mode, short accel, short decel){
	FOREACH_MOTOR(motors[i]->setSpeedProfile(mode, accel, decel));
}
