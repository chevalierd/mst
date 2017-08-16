#include "Servo.h"
#include "Conceptinetics.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

using namespace std;

// twelve servo objects can be created on most boards
const int DMX_SLAVE_CHANNELS = 3;

enum SwitchPosition {NEUTRAL, ON, OFF};

const int MOVE_DELAY = 2500;  // delay in miliseconds before returning to neutral position

class DMXSwitch {
public:
  DMXSwitch(const int slaveChannels) : dmx_(slaveChannels) {
    lastValues_ = new SwitchPosition[slaveChannels]();
      dmx_.enable();
  }

  ~DMXSwitch() {
    delete [] lastValues_;
  }

  SwitchPosition getSwitchFromDmx(int offset) {
    const int dmxValue = dmx_.getChannelValue(offset);
    if(dmxValue > 2 && dmxValue  < 127) {
      lastValues_[offset] = OFF;
      return OFF;
    }
    if(dmxValue >= 127) {
      lastValues_[offset] = ON;
      return ON;
    }
    return lastValues_[offset];
  }

  void setStartAddress(const int start) {
    // Set start address
    // You can change this address at any time during the program
    dmx_.setStartAddress(start);
  }
  DMX_Slave& getDmx() { return dmx_; }
private:
  DMX_Slave dmx_;
  SwitchPosition* lastValues_;
};

class ServoSwitch {
public:
  ServoSwitch(const int id, const int pin)
    : id_(id)
    , switchPosition_(NEUTRAL)
    , availableToMoveAgain_(0)
    , pendingMoveToNeutral_(0) {
      servo_.attach(pin); // Attach servos to pins starting at 3
      moveNeutral();  // Start servo at neutral position
  }

  void doCycle(const SwitchPosition proposedPosition, const unsigned long currentMillis) {
    handlePendingNeutral(currentMillis);
    maybeDmxMoveServo(proposedPosition, currentMillis);
  }

private:
  void handlePendingNeutral(const unsigned long currentMillis) {
    if(availableToMoveAgain_ != 0 && availableToMoveAgain_ <  currentMillis) {
      // delay time has elapsed, we can move again
      cout << "Clearing pending move servo=" << id_ << endl;
      availableToMoveAgain_ = 0;
      pendingMoveToNeutral_ = 0;
    } else if(pendingMoveToNeutral_!= 0 && pendingMoveToNeutral_ < currentMillis && availableToMoveAgain_==0) {
        // move neutral and setup a timer to make sure servo completes neutral
        availableToMoveAgain_ = currentMillis + MOVE_DELAY;
        moveNeutral();
        cout << "Moving to neutral after position change servo=" << id_ << endl;
    } else {
      cout <<"no pending servo=" << id_ << endl;
    }
  }

  void maybeDmxMoveServo(const SwitchPosition proposedPosition, const unsigned long currentMillis) {
    if(canDmxMoveServo(proposedPosition)) {
      pendingMoveToNeutral_ = currentMillis + MOVE_DELAY;
      cout <<"pendingMoveToNeutral[" << id_ << "]" << "=" << pendingMoveToNeutral_ << endl;
      switch(proposedPosition) {
        case ON:
          moveOn();
          break;
        case OFF:
          moveOff();
          break;
        case NEUTRAL:
          // Don't want dmx to cause a movement
          break;
      }
    }
  }

  bool canDmxMoveServo(const SwitchPosition proposedPosition) {
    // DMX cannot move to neutral
    if(proposedPosition == NEUTRAL) {
      cout << "cannot move to neutral from dmx servo=" <<id_ << endl;
      return false;
    }
    // Move if this is a new position and no movements are in progress.
    if(proposedPosition != switchPosition_) {
      if(pendingMoveToNeutral_==0) {
        return true;
      } else {
        cout << "cannot move during pending neutral servo=" << id_ << endl;
        return false;
      }
    } else {
      cout << "no need to move servo=" << id_ << endl;
      return false;
    }
  }

  void moveNeutral() {
    servo_.write(90);
    cout << "moveNeutral servo=" << id_ << endl;
  }

  void moveOff() {
    servo_.write(180);
    switchPosition_ = OFF;
    cout << "moveOff servo=" << id_ << endl;
  }

  void moveOn() {
    servo_.write(0);
    switchPosition_ = ON;
    cout << "moveOn servo=" << id_ << endl;
  }

  int id_;
  Servo servo_;
  SwitchPosition switchPosition_;
  unsigned long availableToMoveAgain_;
  unsigned long pendingMoveToNeutral_;
};


DMXSwitch* gDmxSwitch(nullptr);
std::vector<ServoSwitch*> gServoSwitches;;

void setup() {
  gDmxSwitch = new DMXSwitch(DMX_SLAVE_CHANNELS);
  // Set start address
  // You can change this address at any time during the program
  gDmxSwitch->setStartAddress(501);

  for(int i=0 ; i < DMX_SLAVE_CHANNELS ; ++i) {
    gServoSwitches.push_back(new ServoSwitch(i, i+3));
  }
}

void loop() {
  unsigned long currentMillis = millis();
  cout << "currentMillis " << currentMillis << endl;
  std::for_each(gServoSwitches.begin(), gServoSwitches.end(),
    [*](ServoSwitch* servoSwitch) {
      servoSwitch->doCycle(gDmxSwitch->getSwitchFromDmx(i), currentMillis);
    });
}

int main() {

  setup();
  cout << "-----" << endl;
  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;
  gDmxSwitch->getDmx().setChannelValue(0, 50);
  gDmxSwitch->getDmx().setChannelValue(1, 128);

  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;

  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;

  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;

  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;

  gDmxSwitch->getDmx().setChannelValue(0, 200);
  gDmxSwitch->getDmx().setChannelValue(1, 5);

  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;

  gDmxSwitch->getDmx().setChannelValue(1, 200);

  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;

  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;

  loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  cout << "-----" << endl;

}
