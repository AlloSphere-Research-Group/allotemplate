#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

class MyApp : public App {
public:
  ShaderProgram shader;
  VAOMesh mesh;
  Graphics g;
  osc::Recv server { 16447, "", 0.05 };

  void onInit() {
    cout << "after glfw init, before window creation" << endl;
  }
  
  void onCreate() {
    string const vert_source = R"(
      #version 330

      uniform mat4 m;

      layout (location = 0) in vec4 position;
      layout (location = 1) in vec4 color;
      layout (location = 2) in vec2 texcoord;

      out vec4 _color;

      void main() {
        gl_Position = m * position;
        _color = color;
      }
    )";

    string const frag_source = R"(
      #version 330

      in vec4 _color;

      out vec4 frag_color;

      void main() {
        frag_color = _color;
      }
    )";

    shader.compile(vert_source, frag_source);

    mesh.reset();
    mesh.primitive(TRIANGLES);
    mesh.vertex(-0.5, -0.5, 0);
    mesh.color(1.0, 0.0, 0.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.vertex(-0.5, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.vertex(-0.5, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.vertex(0.5, 0.5, 0);
    mesh.color(0.0, 1.0, 1.0);
    mesh.update(); // send to gpu buffers

    g.setClearColor(0, 1, 1);

    server.handler(*this);
    server.start();
  }

  void onAnimate(double dt) {

  }

  void onDraw() {
    g.viewport(0, 0, fbWidth(), fbHeight());
    g.clear();

    float w = width();
    float h = height();

    // simple projection matrix for now
    auto proj = Matrix4f::scaling(h / w, 1.0f, 1.0f);

    Matrix4f mat = Matrix4f::rotate(sec(), 0, 0, 1);
    mat = Matrix4f::scaling(h / w, 1.0f, 1.0f) * mat;
    
    shader.begin();

    g.pushMatrix();
    g.rotate(sec(), 0, 0, 1);
    g.translate(-0.5, 0, 0);
    shader.uniform("m", proj * g.modelMatrix());
    mesh.draw();
    g.popMatrix();

    g.pushMatrix();
    g.rotate(2 * sec(), 0, 0, 1);
    g.translate(0.5, 0, 0);
    shader.uniform("m", proj * g.modelMatrix());
    mesh.draw();
    g.popMatrix();

    shader.end();
  }

  void onMessage(osc::Message& m) {
    if (m.addressPattern() == "/test") {
        // Extract the data out of the packet
        std::string str;
        m >> str;

        // Print out the extracted packet data
        std::cout << "SERVER: recv " << str << endl;
    }
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
      float out1 = cos(5*phase * 2*M_PI);
      float out2 = sin(4*phase * 2*M_PI);

      // Send scaled waveforms to output...
      io.out(0) = out1*0.2;
      io.out(1) = out2*0.4;
    }
  }

  void onExit() {
    cout << "onExit" << endl;
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
  app.start(); // blocks
  return 0;
}