#include "al/core.hpp"
#include "cubemap.hpp"
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

int const cuberes {512};

Mat4f get_cube_mat(int face) {
  switch (face) {
    // GL_TEXTURE_CUBE_MAP_POSITIVE_X
    case 0: return Mat4f {
      0, 0,-1, 0,
      0,-1, 0, 0,
      -1, 0, 0, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    case 1: return Mat4f {
      0, 0, 1, 0,
      0, -1, 0, 0,
      1, 0, 0, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    case 2: return Mat4f {
      1, 0, 0, 0,
      0, 0, 1, 0,
      0, -1, 0, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    case 3: return Mat4f {
      1, 0, 0, 0,
      0, 0, -1, 0,
      0, 1, 0, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    case 4: return Mat4f {
      1, 0, 0, 0,
      0, -1, 0, 0,
      0, 0, -1, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    case 5: return Mat4f {
      -1, 0, 0, 0,
      0, -1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    };
  }
  return Mat4f::identity();
}

class MyApp : public App {
public:
  Viewpoint cube_view;
  NavInputControl nav;
  ShaderProgram shader;
  VAOMesh mesh;
  FBO fbo;
  CubeMap cubemap;
  RBO rbo;
  Viewpoint stationary_view;
  ShaderProgram cubeshader;
  ShaderProgram cubesampleshader;
  VAOMesh quad;

  void onCreate() {
    append(nav.target(cube_view));
    
    shader.compile(al_default_vert_shader(), al_default_frag_shader());
    shader.begin();
    shader.uniform("tex0", 0);
    shader.uniform("tex0_mix", 0.0);
    shader.uniform("light_mix", 0.0f);
    shader.end();

    addIcosahedron(mesh);
    auto num_verts = mesh.vertices().size();
    for (int i = 0; i < num_verts; i++) {
      mesh.color(i / float(num_verts), (num_verts - i) / float(num_verts), 0.0);
    }
    mesh.update();

    cube_view.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    cube_view.fovy(90).near(0.1).far(100);
    cube_view.viewport(0, 0, cuberes, cuberes);

    cubemap.init(cuberes);

    stationary_view.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    stationary_view.fovy(30).near(0.1).far(100);
    stationary_view.viewport(0, 0, fbWidth(), fbHeight());

    rbo.create(cuberes, cuberes);
    fbo.bind();
    glFramebufferTexture2D(
      GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_CUBE_MAP_POSITIVE_X,
      cubemap.id(),
      0 // level
    );
    fbo.unbind();
    fbo.attachRBO(rbo);
    printf("fbo status %s\n", fbo.statusString());

    cubeshader.compile(cubevert(), cubefrag());
    cubeshader.begin();
    cubeshader.uniform("tex0", 0);
    cubeshader.uniform("tex0_mix", 0.0);
    cubeshader.uniform("light_mix", 0.0f);
    cubeshader.end();

    cubesampleshader.compile(cubesamplevert(), cubesamplefrag());
    cubesampleshader.begin();
    cubesampleshader.uniform("cubemap", 1);
    cubesampleshader.end();

    quad.reset();
    quad.primitive(Mesh::TRIANGLES);
    quad.vertex(-3, -1.5, 0);
    quad.texCoord(0.0, 0.0);
    quad.vertex(3, -1.5, 0);
    quad.texCoord(1.0, 0.0);
    quad.vertex(-3, 1.5, 0);
    quad.texCoord(0.0, 1.0);
    quad.vertex(-3, 1.5, 0);
    quad.texCoord(0.0, 1.0);
    quad.vertex(3, -1.5, 0);
    quad.texCoord(1.0, 0.0);
    quad.vertex(3, 1.5, 0);
    quad.texCoord(1.0, 1.0);
    quad.update(); // send to gpu buffers
  }

  void onAnimate(double dt) {
      nav.step();
  }

  void onDraw() {
    auto& g = graphics();

    fbo.bind();

    g.polygonMode(Graphics::FILL);
    g.depthTesting(true);
    g.cullFace(true); // default front face is CCW, default cull face is BACK

    g.shader(cubeshader);
    g.camera(cube_view);
    g.shader().uniform("omni_eyeSep", 0.01);
    g.shader().uniform("omni_radius", 1e10);

    for (int j = 0; j < 6; j++){
      g.shader().uniform("C", get_cube_mat(j));

      glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X+j,
        cubemap.id(), 0
      );
      g.clear(j / 5.0, (5 - j) / 5.0, 0);
      g.clearDepth(1);

      g.pushMatrix();
      g.translate(sinf(sec()), 0, 0);
      g.rotate(sinf(2 * sec()), 0, 0, 1);
      g.rotate(sinf(3 * sec()), 0, 1, 0);
      g.scale(3, 2, 1);
      g.draw(mesh);
      g.popMatrix();
    }

    fbo.end();

    g.clear(0, 0, 0);
    g.clearDepth(1);

    g.shader(cubesampleshader);
    g.camera(stationary_view);

    g.polygonMode(Graphics::FILL);
    g.depthTesting(true);
    g.cullFace(true); // default front face is CCW, default cull face is BACK

    cubemap.bind(1);
    g.draw(quad);
    cubemap.unbind(1);

    g.shader(shader);
    g.pushMatrix();
    g.translate(sinf(sec()), 0, 0);
    g.rotate(sinf(2 * sec()), 0, 0, 1);
    g.rotate(sinf(3 * sec()), 0, 1, 0);
    g.scale(0.5, 0.5, 1);
    g.draw(mesh);
    g.popMatrix();

  }

  void onResize(int w, int h) {
    stationary_view.viewport(0, 0, fbWidth(), fbHeight());
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