#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

string al_default_vert_shader() {
  return R"(#version 330
uniform mat4 MVP;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 normal;

out vec4 color_;
out vec2 texcoord_;
out vec3 normal_;

void main() {
  gl_Position = MVP * vec4(position, 1.0);
  color_ = color;
  texcoord_ = texcoord;
  normal_ = normal;
})";
}

string al_default_frag_shader() {
  return R"(#version 330
uniform sampler2D tex0;
uniform float tex0_mix;
uniform float light_mix;
in vec4 color_;
in vec2 texcoord_;
in vec3 normal_;
out vec4 frag_color;
void main() {
  vec4 tex_color0 = texture(tex0, texcoord_);
  vec4 light_color = vec4(normal_, 1.0); // TODO
  vec4 final_color = mix(mix(color_, tex_color0, tex0_mix), light_color, light_mix);
  frag_color = final_color;
})";
}

class MyApp : public App {
public:
  Viewpoint viewpoint;
  NavInputControl nav;
  ShaderProgram shader;
  Texture texture;
  VAOMesh mesh, mesh2;

  void onCreate() {
    append(nav.target(viewpoint));
    
    shader.compile(al_default_vert_shader(), al_default_frag_shader());
    // shader.listParams();

    texture.create2D(16, 16, GL_RGBA8, GL_RGBA, GL_FLOAT);
    array<float, 16 * 16 * 4> arr;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i + 16 * j;
            arr[4 * idx + 0] = float(i) / 15.0f;
            arr[4 * idx + 1] = float(j) / 15.0f;
            arr[4 * idx + 2] = 0.0f;
            arr[4 * idx + 3] = 1.0f;
        }
    }

    texture.bind();
    texture.submit(arr.data()); // give raw pointer
    texture.mipmap(false); // false by default. turn on only if needed
    texture.update();
    texture.unbind();

    generate_mesh();

    addIcosahedron(mesh2);
    auto num_verts = mesh2.vertices().size();
    for (int i = 0; i < num_verts; i++) {
      mesh2.color(i / float(num_verts), (num_verts - i) / float(num_verts), 0.0);
    }
    mesh2.update();

    viewpoint.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    viewpoint.fovy(30).near(0.1).far(100);
    viewpoint.viewport(0, 0, fbWidth(), fbHeight());
  }

  void onAnimate(double dt) {
      nav.step();
  }

  void onDraw() {
    auto& g = graphics();
    g.clear(0, 1, 1);
    g.clearDepth(1);

    // g.polygonMode(Graphics::POINT);
    // g.pointSize(10);
    // g.polygonMode(Graphics::LINE);
    g.polygonMode(Graphics::FILL);
    g.depthTesting(true);
    g.cullFace(true); // default front face is CCW, default cull face is BACK

    g.shader(shader);
    g.camera(viewpoint);
    g.texture(texture);

    g.shader().uniform("tex0", 0);
    g.shader().uniform("tex0_mix", 0.5 + 0.5 * sinf(sec()));
    g.shader().uniform("light_mix", 0.0f);

    g.pushMatrix();
    g.translate(-1, 0, 0);
    g.rotate(sec(), 0, 0, 1);
    g.scale(3, 2, 1);
    g.draw(mesh);
    g.popMatrix();

    g.shader().uniform("tex0_mix", 0.0);
    g.pushMatrix();
    g.translate(1, 0, 0);
    g.rotate(2 * sec(), 0, 0, 1);
    g.draw(mesh2);
    g.popMatrix();
  }

  void generate_mesh() {
    mesh.reset();
    mesh.primitive(Mesh::TRIANGLES);
    mesh.vertex(-0.5, -0.5, 0);
    mesh.color(1.0, 0.0, 0.0);
    mesh.texCoord(0.0, 0.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.texCoord(1.0, 0.0);
    mesh.vertex(-0.5, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.texCoord(0.0, 1.0);
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