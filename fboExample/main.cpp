#include "al/core.hpp"
#include <array>

using namespace al;
using namespace std;

float const w = 256;
float const h = 256;

class MyApp : public EasyApp {
public:
  EasyFBO fbo;
  Viewpoint fboView; // camera to use when drawing to fbo
  Mesh mesh;
  Mesh texRect; // to display fbo texture

  void onCreate() {
    fbo.init(w, h);

    addIcosahedron(mesh);

    fboView.pos(Vec3f(0, 0, 10));
    fboView.viewport(0, 0, w, h); // same as our fbo
    // make keyboard navigation control fboView
    nav.target(fboView);

    addTexRect(texRect, 50, 50, 500, 500);
  }

  void onAnimate(double dt) {

  }

  void onDraw() {
    g.framebuffer(fbo);
    g.clear(0, 1, 1);
    g.shader(color_shader);
    g.shader().uniform("col0", Color(1, 0, 0));
    g.camera(fboView);
    g.draw(mesh);

    g.framebuffer(FBO::DEFAULT);
    g.clear(0, 0, 1);
    g.shader(tex_shader);
    g.camera(Viewpoint::ORTHO_FOR_2D); // for 2D display of fbo texture
    fbo.tex().bind(0);
    g.shader().uniform("tex0", 0);
    g.draw(texRect);
    fbo.tex().unbind(0);
  }

};

int main() {
  MyApp app;
  app.title("fbo example");
  app.dimensions(800, 600);
  app.start();
  return 0;
}