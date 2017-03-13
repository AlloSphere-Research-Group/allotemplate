#include "al/core.hpp"
#include <iostream>
#include <string>

using namespace al;
using namespace std;

class MyApp : public App {
public:
  Viewpoint viewpoint;
  NavInputControl nav;
  ShaderProgram shader;
  VAOMesh mesh;
  Graphics g;

  void onCreate() {
    append(nav.target(viewport));

    string const vert_source = R"(
      #version 330
      uniform mat4 MVP;

      layout (location = 0) in vec4 position;
      layout (location = 1) in vec4 color;

      out vec4 color_;

      void main() {
        gl_Position = MVP * position;
        color_ = color;
      }
    )";

    string const frag_source = R"(
      #version 330
      in vec4 color_;
      out vec4 frag_color;
      void main() {
        frag_color = color_;
      }
    )";

    shader.compile(vert_source, frag_source);

    generate_mesh();

    viewpoint.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    viewpoint.fovy(30).near(0.1).far(100);
    viewpoint.viewport(0, 0, fbWidth(), fbHeight());
  }

  void onAnimate(double dt) {
      nav.step();
  }

  void onDraw() {
    g.clear(0, 1, 1);

    // g.polygonMode(POINT);
    // g.pointSize(10);
    // g.polygonMode(LINE);
    g.polygonMode(FILL);

    g.shader(shader);
    g.camera(viewpoint);

    g.pushMatrix();
    g.translate(-1, 0, 0);
    g.rotate(sec(), 0, 0, 1);
    g.scale(3, 2, 1);
    g.draw(mesh);
    g.popMatrix();

    g.pushMatrix();
    g.translate(1, 0, 0);
    g.rotate(2 * sec(), 0, 0, 1);
    g.draw(mesh);
    g.popMatrix();

  }

  void generate_mesh() {
      mesh.reset();
      mesh.primitive(TRIANGLES);
      mesh.vertex(-0.5, -0.5, 0);
      mesh.color(1.0, 0.0, 0.0);
      mesh.vertex(0.5, -0.5, 0);
      mesh.color(0.0, 1.0, 0.0);
      mesh.vertex(-0.5, 0.5, 0);
      mesh.color(0.0, 0.0, 1.0);
      mesh.update(); // send to gpu buffers
  }

  void onResize(int w, int h) {
    viewpoint.viewport(0, 0, fbWidth(), fbHeight());
  }
};

int main() {
  MyApp app;
  app.dimensions(640, 480);
  app.title("app test");
  app.fps(60);
  app.decorated(true);
  app.displayMode(Window::DEFAULT_BUF);
  app.start(); // blocks
  return 0;
}