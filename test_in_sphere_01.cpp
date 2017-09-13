#include "al/core.hpp"

#include <cmath>

using namespace al;
using namespace std;

class MyApp : public App {
public:
  Graphics g {this};

  void onInit()
  {
    // GLFWmonitor* primary = glfwGetPrimaryMonitor();
    // const GLFWvidmode* mode = glfwGetVideoMode(primary);
    // dimensions(0, 0, mode->width, mode->height);

    // Correct Window Size in AlloSphere
    // (glfw can't see mosaic)
    // Donghao's awesome window size finder
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    // printf("monitor count: %d\n", count);

    int width = 0;
    int height = 0;
    for(int i = 0; i < count; i++) {
        int x, y;
        glfwGetMonitorPos(monitors[i], &x, &y);
        const GLFWvidmode* vm = glfwGetVideoMode(monitors[i]);
        int xmax = x + vm->width;
        int ymax = y + vm->height;
        if(width < xmax) width = xmax;
        if(height < ymax) height = ymax;
    }

    dimensions(0, 0, width, height);
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