#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

// example of 2D & 3D mix with fbo

int const w = 400;
int const h = 300;

class MyApp : public App {
public:
  Viewpoint viewpoint;
  NavInputControl nav;
  ShaderProgram mesh_shader;
  ShaderProgram color_shader;
  ShaderProgram tex_shader;
  VAOMesh mesh_2d, mesh_3d, texRect;
  Graphics g {this};
  EasyFBO fbo;

  void onCreate() {
    // viewpoint: pose of camera, lens of camera, and viewport size
    viewpoint.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    viewpoint.fovy(30).near(0.1).far(100);
    viewpoint.viewport(0, 0, w, h); // same as our fbo
    append(nav.target(viewpoint));

    mesh_shader.compile(al_mesh_vert_shader(), al_mesh_frag_shader());
    color_shader.compile(al_color_vert_shader(), al_color_frag_shader());
    tex_shader.compile(al_tex_vert_shader(), al_tex_frag_shader());

    fbo.init(w, h);

    // 2D triangles
    mesh_2d.primitive(Mesh::TRIANGLES);
    mesh_2d.vertex(0, 0, 0);
    mesh_2d.vertex(70, 0, 0);
    mesh_2d.vertex(0, 70, 0);
    mesh_2d.update(); // to do this is important: uploads mesh data to VAO

    // 3D volume
    addIcosahedron(mesh_3d);    
    int num_verts = mesh_3d.vertices().size();
    for (int i = 0; i < num_verts; i++) {
      mesh_3d.color(i / float(num_verts), (num_verts - i) / float(num_verts), 1.0);
    }
    mesh_3d.update();

    // to display fbo texture
    addTexRect(texRect, width() / 2 - 320, height() / 2 - 240, 640, 480);
    texRect.update();
  }

  void onAnimate(double dt) {
    nav.step();
  }

  void onDraw() {

    // draw 3D to offscreen
    g.framebuffer(fbo);
    g.clear(0, 1, 1);
    g.shader(mesh_shader);
    g.camera(viewpoint);

    g.blending(false);
    g.depthTesting(true);
    g.cullFace(true);

    g.pushMatrix();
    g.translate(sinf(sec()), 0, -10);
    g.rotate(sinf(2 * sec()), 0, 0, 1);
    g.rotate(sinf(3 * sec()), 0, 1, 0);
    g.scale(4, 3, 4);
    g.draw(mesh_3d);
    g.popMatrix();


    g.framebuffer(FBO::DEFAULT);
    // now do 2d drawing to window's default framebuffer
    g.camera(Viewpoint::ORTHO_FOR_2D);
    g.clear(0.5, 0.5, 0.5);

    // setting for 2D drawing
    g.blending(true);
    g.blendModeTrans();
    g.depthTesting(false);
    g.cullFace(false);

    // draw 3D scene
    g.shader(tex_shader);
    fbo.tex().bind(0);
    g.shader().uniform("tex0", 0);
    g.draw(texRect);
    fbo.tex().unbind(0);

    // then draw overlaying 2D triangles
    g.shader(color_shader);
    g.polygonMode(Graphics::FILL);
    for (int j = 0; j < 3; j += 1) {
      for (int i = 0; i < 4; i += 1) {
        g.pushMatrix();
        g.translate(50 + i * 150, 50 + j * 150);
        g.shader().uniform("col0", Color(i / 3.0f, j / 2.0f, 0.5));
        g.draw(mesh_2d);
        g.popMatrix();
      }
    }
  }

};

int main() {
  MyApp app;
  app.dimensions(720, 640);
  app.start();
  return 0;
}