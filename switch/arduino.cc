#include <Servo.h>
#include <Conceptinetics.h>


// twelve servo objects can be created on most boards
const int DMX_SLAVE_CHANNELS = 10;
Servo servoList[DMX_SLAVE_CHANNELS];

// Configure a DMX slave controller
DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS );

enum SwitchPosition {NEUTRAL, ON, OFF};
SwitchPosition currentPosition[DMX_SLAVE_CHANNELS]={NEUTRAL};
unsigned long availableToMoveAgain[DMX_SLAVE_CHANNELS]={0};
unsigned long pendingMoveToNeutral[DMX_SLAVE_CHANNELS]={0};
const int MOVE_DELAY = 2500;  // delay in miliseconds before returning to neutral position

void moveOff(int servo) {
  if(servo < DMX_SLAVE_CHANNELS) {
    servoList[servo].write(180);
    currentPosition[servo] = OFF;
  }
}

void moveOn(int servo) {
  if(servo < DMX_SLAVE_CHANNELS) {
    servoList[servo].write(0);
    currentPosition[servo] = ON;
  }
}

void moveNeutral(int servo) {
  if(servo < DMX_SLAVE_CHANNELS) {
    servoList[servo].write(90);
  }
}

void setup() {
  for(int i=0 ; i < DMX_SLAVE_CHANNELS ; ++i) {
    servoList[i].attach(3+i); // Attach servos to pins starting at 3
    moveNeutral(i);  // Start servo at neutral position
    currentPosition[i] = NEUTRAL;
  }

  // Enable DMX slave interface and start recording
  // DMX data
  dmx_slave.enable();

  // Set start address
  // You can change this address at any time during the program
  dmx_slave.setStartAddress (501);
}


SwitchPosition getSwitchFromDmx(int offset) {
  const int dmxValue = dmx_slave.getChannelValue(offset);
  if(dmxValue > 2 && dmxValue  < 127) {
    return OFF;
  }
  if(dmxValue >= 127) {
    return ON;
  }
  return NEUTRAL;
}

void handlePendingNeutral(int servo, unsigned long currentMillis) {
  if(availableToMoveAgain[servo] != 0 && availableToMoveAgain[servo] <  currentMillis) {
    // delay time has elapsed, we can move again
    availableToMoveAgain[servo] = 0;
    pendingMoveToNeutral[servo] = 0;
  } else if(pendingMoveToNeutral[servo]!= 0 && pendingMoveToNeutral[servo] < currentMillis && availableToMoveAgain[servo]==0) {
      // move neutral and setup a timer to make sure servo completes neutral
      availableToMoveAgain[servo] = currentMillis + MOVE_DELAY;
      moveNeutral(servo);
  }
}

bool canDmxMoveServo(int servo, SwitchPosition proposedPosition) {
  // DMX cannot move to neutral
  if(proposedPosition == NEUTRAL) {
    return false;
  }
  // Move if this is a new position and no movements are in progress.
  return proposedPosition != currentPosition[servo] && pendingMoveToNeutral[servo]==0;
}

void maybeDmxMoveServo(int servo, SwitchPosition proposedPosition, unsigned long currentMillis) {
  if(canDmxMoveServo(servo, proposedPosition)) {
    pendingMoveToNeutral[servo] = currentMillis + MOVE_DELAY;
    switch(proposedPosition) {
      case ON:
        moveOn(servo);
        break;
      case OFF:
        moveOff(servo);
        break;
      case NEUTRAL:
        // Don't want dmx to cause a movement
        break;
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();
  for (int servo = 0; servo < DMX_SLAVE_CHANNELS; ++servo){
    handlePendingNeutral(servo, currentMillis);
    SwitchPosition proposedPosition = getSwitchFromDmx(servo);
    maybeDmxMoveServo(servo, proposedPosition, currentMillis);
  }
}
