
// Include the header file for this source file. Use double quotes to select
// from the directory containing this file.
#include "app.hpp"

// Include library headers that are not required in this source file's header.
#include <Gamma/AudioIO.h>

namespace application_name {
void App::onCreate() {
  // Match the Gamma sampling rate to Allolib's audio.
  gam::sampleRate(audioIO().framesPerSecond());

  // Manually initialize our demo.
  singingPolyhedron.init();


  // Configure the camera.
  lens().near(0.1).far(25).fovy(45);
  nav().pos(0, 0, 4);
  nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);
}

void App::onAnimate(double dt) {
  // Manually update the singing polyhedron's animation.
  singingPolyhedron.update(dt);
}

void App::onDraw(al::Graphics &g) {
  // Forward graphics calls to the singing polyhedron.
  singingPolyhedron.onProcess(g);
}

void App::onSound(al::AudioIOData &io) {
  // Forward audio calls to the singing polyhedron.
  singingPolyhedron.onProcess(io);
}

}; // namespace application_name
