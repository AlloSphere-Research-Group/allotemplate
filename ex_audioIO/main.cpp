#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

// example of audio output

class MyApp : public App {
public:
  Graphics g {*this};

  void onCreate() {

  }

  void onDraw() {
    g.clearColor(0, 0, 0);
  }
  
  void onSound(AudioIOData& io) {
    static double phase {0};
    // Set the base frequency to 55 Hz
    double freq = 55/io.framesPerSecond();

    while(io()){
      // Update the oscillators' phase
      phase += freq;
      if(phase > 1) phase -= 1;

      // Generate two sine waves at the 5th and 4th harmonics
      float out = 0.3 * cos(5*phase * 2*M_PI);

      // Send scaled waveforms to output...
      io.out(0) = out;
      io.out(1) = out;
    }
  }

};

int main() {
  MyApp app;
  app.initAudio();
  app.start(); // blocks
  return 0;
}