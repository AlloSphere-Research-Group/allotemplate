#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

// app interface
// for audio, goto 'ex_audioIO'

bool first_frame = true;

class MyApp : public App {
public:
  Graphics g {*this};

  void onInit() {
    cout << "onInit: after glfw init, before window creation" << endl;
    // so you can do things like
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    // change dimensions depending on the monitor resolution
    dimensions(mode->width / 4, mode->height / 2);
  }

  void onCreate() {
    cout << "onCreate: after window creation" << endl;
  }

  void onAnimate(double dt) {
    if (first_frame) cout << "onAnimate: every frame before draw" << endl;
  }

  void onDraw() {
    if (first_frame) cout << "onDraw: every frame for draw calls" << endl;
    g.clearColor(0, 0, 0);
    first_frame = false;
  }

  void onKeyDown(Keyboard const& k) {
    cout << "a key went down: " << k.key() << endl;
  }

  void onKeyUp(Keyboard const& k) {
    cout << "a key went up: " << k.key() << endl;
  }

  void onResize(int w, int h) {
    cout << "window is resized" << endl;
  }

  void onExit() {
    cout << "onExit: before assets are destroyed" << endl;
  }

  void onMouseDown(Mouse const& m) {
    cout << "mouse down" << endl;
  }

  void onMouseUp(Mouse const& m) {
    cout << "mouse up" << endl;
  }

  void onMouseDrag(Mouse const& m) {
    static int i = 0;
    if (i == 0) cout << "mouse dragged" << endl;
    i++;
    i = i % 20;
  }

  void onMouseMove(Mouse const& m) {
    static int i = 0;
    if (i == 0) cout << "mouse moved" << endl;
    i++;
    i = i % 40;
  }

};

int main() {
  MyApp app;
  app.initAudio();
  app.dimensions(640, 480);
  app.title("app test");
  app.fps(60);
  app.decorated(true);
  app.displayMode(Window::DEFAULT_BUF);
  app.start();
  return 0;
}