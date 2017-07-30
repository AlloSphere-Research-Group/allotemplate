#include "al/core.hpp"

using namespace al;
using namespace std;

float const w = 256;
float const h = 256;

class MyApp : public EasyApp {
public:
  EasyFBO fbo;
  Viewpoint fboView;
  Mesh mesh;

  void onCreate() {
    fbo.init(w, h);

    addIcosahedron(mesh);

    // camera to use when drawing to fbo
    fboView.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    fboView.fovy(30).near(0.1).far(100);
    fboView.viewport(0, 0, w, h); // same as our fbo

    // make keyboard navigation control fboView
    nav.target(fboView);
  }

  void onAnimate(double dt) {

  }

  void onDraw() {
    g.framebuffer(fbo);
    g.clear(0, 1, 0);

    g.camera(fboView);
    g.uniformColorMix(1);
    g.uniformColor(1, 0, 0);
    g.textureMix(0);
    g.draw(mesh);

    g.framebuffer(FBO::DEFAULT);
    g.clear(0, 0, 1);
    
    g.camera(Viewpoint::ORTHO_FOR_2D); // for 2D display of fbo texture
    g.draw(fbo, 50, 50, width() - 100, height() - 100);
  }

};

int main() {
  MyApp app;
  app.title("fbo example");
  app.dimensions(800, 600);
  app.start();
  return 0;
}