#include "al/core.hpp"

#include <cmath>

using namespace al;
using namespace std;

class MyApp : public App {
public:
  Graphics g {this};

  void onInit()
  {
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    dimensions(0, 0, mode->width, mode->height);
  }

  void onCreate()
  {

  }

  void onAnimate(double dt)
  {

  }

  void onDraw()
  {
    float t = frameCount() * 0.01;
    g.clearColor(sin(13 * t), sin(17 * t), sin(19 * t));
  }

};

int main() {
  MyApp app;
  app.decorated(false);
  app.start();
  return 0;
}