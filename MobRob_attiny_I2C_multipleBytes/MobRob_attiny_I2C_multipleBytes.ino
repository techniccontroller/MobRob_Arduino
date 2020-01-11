/*
 * Created by Techniccontroller 2019
 */

#include <Arduino.h>
#include <TinyWireS.h>
#include <TinyServo.h>

#define SERVO_PIN 9
#define I2C_SLAVE_ADDRESS 0x5

int index = 0;




TinyServo servo(SERVO_PIN);

// global buffer to store data sent from the master.
uint8_t master_data[32];
// global variable to number of bytes sent from the master.
uint8_t master_bytes;

// global buffer to store data sent to the master.
uint8_t slave_data[32];
// global buffer to store data sent to the master.
uint8_t slave_bytes = 0;
int new_data_available = 1;

// Gets called when the ATtiny receives an i2c write slave request
// This routine runs from the usiTwiSlave interrupt service routine (ISR)
// so interrupts are disabled while it runs.
void receiveEvent(uint8_t num_bytes)
{
  uint8_t i;

  // save the number of bytes sent from the master
  master_bytes = num_bytes;

  // store the data from the master into the data buffer
  for (i = 0; i < master_bytes; i++)
    master_data[i] = TinyWireS.receive();

  new_data_available = 1;
}


// Gets called when the ATtiny receives an i2c read slave request
// This routine runs from the usiTwiSlave interrupt service routine (ISR)
// so interrupts are disabled while it runs.
void requestEvent()
{
  uint8_t i;
  
  // send the data buffer back to the master
  for (i = 0; i < slave_bytes; i++)
    TinyWireS.send(slave_data[i]);

  slave_data[0] += 1;
}

void process_data(){
  uint8_t command = master_data[0];

  switch(command){
    case 1:
      servo.write(20);
      break;
    case 2:
      servo.write(40);
      break;
    case 3:
      servo.write(master_data[1]);
      break;
    default:
      servo.write(80);
      break;
  }
}

void setup() {
  // initialize the TinyWireS and usiTwiSlave libraries
  TinyWireS.begin(I2C_SLAVE_ADDRESS);      // init I2C Slave mode

  // register the onReceive() callback function
  TinyWireS.onReceive(receiveEvent);
  
  // register the onRequest() callback function
  TinyWireS.onRequest(requestEvent);
  
  servo.write(90);

  // sample data as response
  slave_bytes = 0;
  for(int i = 0; i < 10; i++){
    slave_data[i] = i*2 + 10;
    slave_bytes++;
  }
}

void loop() {

  if (new_data_available){
    process_data();
    new_data_available = 0;
  }
  
  servo.refresh();
  delay(30);
}



