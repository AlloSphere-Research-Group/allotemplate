#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

// example of 2D & 3D mix with fbo

class MyApp : public App {
public:
  Viewpoint viewpoint;
  NavInputControl nav;
  ShaderProgram shader;
  VAOMesh mesh_2d, mesh_3d;
  Graphics g {*this};
  EasyFBO easyfbo;
  Texture tex;
  RBO rbo;

  void onCreate() {
    append(nav.target(viewpoint));
    shader.compile(al_default_vert_shader(), al_default_frag_shader());
    easyfbo.init(640, 480);

    mesh_2d.reset();
    mesh_2d.primitive(Mesh::TRIANGLES);
    mesh_2d.vertex(0, 0, 0);
    mesh_2d.vertex(70, 0, 0);
    mesh_2d.vertex(0, 70, 0);
    mesh_2d.update(); // to do this is important: uploads mesh data to VAO

    addIcosahedron(mesh_3d);
    // addSphere(mesh_3d);
    int num_verts = mesh_3d.vertices().size();
    std::cout << "num verts: " << num_verts << std::endl;
    for (int i = 0; i < num_verts; i++) {
      mesh_3d.color(i / float(num_verts), (num_verts - i) / float(num_verts), 1.0);
    }
    mesh_3d.generateNormals();
    mesh_3d.update();

    // viewpoint: pose of camera, lens of camera, and viewport size
    viewpoint.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    viewpoint.fovy(30).near(0.1).far(100);
    viewpoint.viewport(0, 0, 640, 480); // same as our fbo
  }

  void onAnimate(double dt) {
    nav.step();
  }

  void onDraw() {
    g.shader(shader);

    // draw 3D to offscreen
    g.framebuffer(easyfbo.fbo());
    g.camera(viewpoint);
    g.clearColor(0, 1, 1, 1);
    g.clearDepth(1);

    g.blending(false);
    g.depthTesting(true);
    g.cullFace(true);

    g.textureMix(0);
    g.uniformColorMix(0);

    g.pushMatrix();
    g.translate(sinf(sec()), 0, -10);
    g.rotate(sinf(2 * sec()), 0, 0, 1);
    g.rotate(sinf(3 * sec()), 0, 1, 0);
    g.scale(4, 3, 4);
    g.draw(mesh_3d);
    g.popMatrix();

    // now draw to window
    g.framebuffer(FBO::DEFAULT);
    g.camera(Viewpoint::ORTHO_FOR_2D);
    g.clearColor(0.5, 0.5, 0.5);
    g.clearDepth(1);

    // setting for 2D drawing
    g.blending(true);
    g.blendModeTrans();
    g.depthTesting(false);
    g.cullFace(false);

    // prepare rect to show 3D drawing we did
    VAOMesh m;
    addTexRect(m, width() / 2 - 320, height() / 2 - 240, 640, 480);
    m.update();

    // draw 3D scene
    g.uniformColorMix(0);
    g.textureMix(1);
    g.texture(easyfbo.tex(), 0);
    g.draw(m);

    // then draw overlay 2D
    g.uniformColorMix(1);
    g.textureMix(0);

    g.polygonMode(Graphics::FILL);
    for (int j = 0; j < 3; j += 1) {
      for (int i = 0; i < 4; i += 1) {
        g.pushMatrix();
        g.translate(50 + i * 150, 50 + j * 150);
        g.uniformColor(i / 3.0f, j / 2.0f, 0.5);
        g.draw(mesh_2d);
        g.popMatrix();
      }
    }
  }

};

int main() {
  MyApp app;
  app.dimensions(720, 640);
  app.start(); // blocks
  return 0;
}