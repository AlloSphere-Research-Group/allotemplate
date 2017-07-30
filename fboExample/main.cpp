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
  Mesh texMesh;

  void onCreate() {
    fbo.init(w, h);

    addIcosahedron(mesh);
    addTexRect(texMesh, 50, 50, width() - 100, height() - 100);

    // camera to use when drawing to fbo
    fboView.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    fboView.fovy(30).near(0.1).far(100);
    fboView.viewport(0, 0, w, h); // same as our fbo

    nav.target(fboView);
  }

  void onAnimate(double dt) {

  }

  void onDraw() {
    g.framebuffer(fbo.fbo());
    g.clearColor(0, 1, 0, 1);
    g.clearDepth(1);

    g.camera(fboView);
    g.uniformColorMix(1);
    g.uniformColor(1, 0, 0);
    g.textureMix(0);
    g.draw(mesh);

    g.framebuffer(FBO::DEFAULT);
    g.clearColor(0, 0, 1, 1);
    g.clearDepth(1);
    
    g.camera(Viewpoint::ORTHO_FOR_2D);
    g.uniformColorMix(0);
    g.textureMix(1.0);
    g.texture(fbo.tex());
    g.draw(texMesh);
  }

};

int main() {
  MyApp app;
  app.title("fbo example");
  app.dimensions(800, 600);
  app.start();
  return 0;
}