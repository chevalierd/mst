#include <chrono>

unsigned long millis(){
  using namespace std::chrono;
  milliseconds ms = duration_cast< milliseconds >(
    system_clock::now().time_since_epoch()
  );
  return ms.count();
}

class DMX_Slave {
private:
  const int channels_;
  int baseAddress_;
  int* values_;
public:
  DMX_Slave(const int channels) : channels_(channels) {
    values_ = new int[channels_]();
    baseAddress_ = 0;
  }

  ~DMX_Slave() {
    delete [] values_;
  }
  int getChannelValue(const int channel) {
    return values_[channel];
  }

  void setChannelValue(const int channel, const int value) {
    values_[channel] = value;
  }
  void setStartAddress(const int baseAddress) {
    baseAddress_ = baseAddress;
  }

  void enable(){}
};
