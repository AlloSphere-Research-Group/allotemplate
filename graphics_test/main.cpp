#include "al/core.hpp"
#include <iostream>
#include <string>

using namespace al;
using namespace std;

class MyApp : public App {
public:
  ShaderProgram shader;
  VAOMesh mesh;
  Graphics g;

  void onCreate() {
    string const vert_source = R"(
      #version 330
      uniform mat4 m;
      layout (location = 0) in vec4 position;
      layout (location = 1) in vec4 color;
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

    generate_mesh();

    g.setClearColor(0, 1, 1);

    nav().pos(Vec3d(0, 0, 10)).faceToward(Vec3d(0, 0, 0), Vec3d(0, 1, 0));
    viewport().set(0, 0, fbWidth(), fbHeight());
    lens().fovy(30).near(0.1).far(100);

  }

  void onDraw() {
    g.viewport(viewport());
    g.clear();

    auto proj = viewpoint().projMatrix();
    auto view = viewpoint().viewMatrix();

    g.shader(shader);

    g.pushMatrix();
    g.translate(-0.5, 0, 0);
    g.rotate(sec(), 0, 0, 1);
    g.scale(3, 2, 1);
    g.shader().uniform("m", proj * view * g.modelMatrix());
    g.draw(mesh);
    g.popMatrix();

    g.pushMatrix();
    g.translate(0.5, 0, 0);
    g.rotate(2 * sec(), 0, 0, 1);
    g.shader().uniform("m", proj * view * g.modelMatrix());
    g.draw(mesh);
    g.popMatrix();

  }

  void onResize(int w, int h) {
    viewport().set(0, 0, fbWidth(), fbHeight());
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

      mesh.vertex(-0.5, 0.5, 0);
      mesh.color(0.0, 0.0, 1.0);

      mesh.vertex(0.5, -0.5, 0);
      mesh.color(0.0, 1.0, 0.0);

      mesh.vertex(0.5, 0.5, 0);
      mesh.color(0.0, 1.0, 1.0);
      
      mesh.update(); // send to gpu buffers
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