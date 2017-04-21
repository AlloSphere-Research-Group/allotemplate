#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

// example of 2D & 3D mix with fbo
// a lot of elements will be hidden later

std::string vert_shader() { return R"(
#version 330
uniform mat4 MVP;
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texcoord;
out vec4 color_;
out vec2 texcoord_;
void main() {
  gl_Position = MVP * vec4(position, 1.0);
  color_ = color;
  texcoord_ = texcoord;
}
)";}

std::string frag_shader() { return R"(
#version 330
uniform sampler2D tex0;
uniform float tex0_mix;
uniform vec4 uniformColor;
uniform float uniformColorMix;
in vec4 color_;
in vec2 texcoord_;
out vec4 frag_color;
void main() {
  vec4 tex0_val = texture(tex0, texcoord_);
  vec4 color_val = mix(color_, uniformColor, uniformColorMix);
  frag_color = mix(color_val, tex0_val, tex0_mix);
}
)";}

class MyApp : public App {
public:
  Viewpoint viewpoint;
  NavInputControl nav;
  ShaderProgram shader;
  VAOMesh mesh_2d, mesh_3d;
  Graphics g {*this};
  FBO fbo;
  Texture tex;
  RBO rbo;

  void onCreate() {
    append(nav.target(viewpoint));
    shader.compile(vert_shader(), frag_shader());

    tex.create2D(width(), height());
    rbo.create(width(), height());
    fbo.bind();
    fbo.attachTexture2D(tex);
    fbo.attachRBO(rbo);
    fbo.unbind();

    mesh_2d.reset();
    mesh_2d.primitive(Mesh::TRIANGLES);
    mesh_2d.vertex(0, 0, 0);
    mesh_2d.vertex(70, 0, 0);
    mesh_2d.vertex(0, 70, 0);
    mesh_2d.update(); // it is important to upload mesh data to VAO

    addIcosahedron(mesh_3d);
    int num_verts = mesh_3d.vertices().size();
    for (int i = 0; i < num_verts; i++) {
      mesh_3d.color(i / float(num_verts), (num_verts - i) / float(num_verts), 0.0);
    }
    mesh_3d.update();

    viewpoint.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    viewpoint.fovy(30).near(0.1).far(100);
    viewpoint.viewport(0, 0, width(), height());
  }

  void onAnimate(double dt) {
    nav.step();
  }

  void onDraw() {
    g.shader(shader);
    g.shader().uniform("tex0", 0);
    g.shader().uniform("tex0_mix", 0.0f);

    // draw 3D to offscreen
    g.framebuffer(fbo);
    g.clearColor(0, 1, 1, 1);
    g.clearDepth(1);
    g.blending(false);

    g.depthTesting(true);
    g.cullFace(true);
    g.camera(viewpoint); 

    g.pushMatrix();
    g.translate(sinf(sec()), 0, -10);
    g.rotate(sinf(2 * sec()), 0, 0, 1);
    g.rotate(sinf(3 * sec()), 0, 1, 0);
    g.scale(3, 2, 1);
    g.uniformColorMix(0);
    g.draw(mesh_3d);
    g.popMatrix();

    // now draw to window
    g.framebuffer(FBO::DEFAULT);
    g.clearColor(1, 1, 1);
    g.clearDepth(1);

    // setting for 2D drawing
    g.blending(true);
    g.blendModeTrans();
    g.depthTesting(false);
    g.cullFace(false);
    g.camera(Viewpoint::ORTHO_FOR_2D);

    // prepare rect to show 3D drawing we did
    VAOMesh m;
    float w = width();
    float h = height();
    addTexRect(m, 0.1 * w, 0.1 * h, 0.8 * w, 0.8 * h);
    m.update();

    // draw 3D scene
    g.shader().uniform("tex0_mix", 1.0f);
    g.texture(tex);
    g.draw(m);

    // then draw overlay 2D
    g.uniformColorMix(1);
    g.shader().uniform("tex0_mix", 0.0f);
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

    // end of onDraw
  }
  
  void onResize(int w, int h) {
    // update viewport
    cout << w << ", " << h << endl;
    viewpoint.viewport(0, 0, width(), height());
    tex.create2D(width(), height());
    rbo.create(width(), height());
    fbo.bind();
    fbo.attachTexture2D(tex);
    fbo.unbind();
  }

};

int main() {
  MyApp app;
  app.dimensions(640, 480);
  app.start(); // blocks
  return 0;
}