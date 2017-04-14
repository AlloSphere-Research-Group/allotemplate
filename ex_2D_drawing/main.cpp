#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

// example of 2D drawing with barebone app
// a lot of elements will be hidden later

// should later be in al_lib as default_2D_vert_shader and default_2d_frag_shader
std::string vert_shader() { return R"(
#version 330
uniform mat4 MVP;
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
out vec4 color_;
void main() {
  gl_Position = MVP * vec4(position, 1.0);
  color_ = color;
}
)";}

std::string frag_shader() { return R"(
#version 330
uniform vec4 uniformColor;
uniform float uniformColorMix;
in vec4 color_;
out vec4 frag_color;
void main() {
  frag_color = mix(color_, uniformColor, uniformColorMix);
}
)";}

class MyApp : public App {
public:
  ShaderProgram shader;
  VAOMesh mesh;
  Graphics g {*this};

  void onCreate() {
    shader.compile(vert_shader(), frag_shader());

    // triangle with size 100
    mesh.reset();
    mesh.primitive(Mesh::TRIANGLES);
    mesh.vertex(0, 0, 0);
    mesh.vertex(100, 0, 0);
    mesh.vertex(0, 100, 0);
    mesh.update();

  }

  void onDraw() {
    g.clearColor(0, 0, 0);
    g.clearDepth(1);

    g.shader(shader);

    // below three calls later can be wrapped into g.draw2D();
    g.depthTesting(false);
    g.cullFace(false);
    g.camera(Viewpoint::ORTHO_FOR_2D);

    g.uniformColorMix(1);
    g.polygonMode(Graphics::FILL);
    for (int j = 0; j < 3; j += 1) {
      for (int i = 0; i < 4; i += 1) {
        g.pushMatrix();
        g.translate(50 + i * 150, 50 + j * 150);
        g.uniformColor(i / 3.0f, j / 2.0f, 0);
        g.draw(mesh);
        g.popMatrix();
      }
    }

    // end of onDraw
  }
  
};

int main() {
  MyApp app;
  app.dimensions(640, 480);
  app.start(); // blocks
  return 0;
}