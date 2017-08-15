#include <iostream>

using namespace std;

class Servo {
public:
  void write(int position) { cout << "*servo " << position << endl;}
  void attach(int pin) {}
};
