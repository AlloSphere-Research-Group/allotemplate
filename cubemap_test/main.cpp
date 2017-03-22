#include "al/core.hpp"
#include "cubemap.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

int const cuberes {512};

class MyApp : public App {
public:
  Viewpoint cube_view;
  NavInputControl nav;
  ShaderProgram shader;
  VAOMesh mesh;
  FBO fbo;
  Texture cubemap;
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

    stationary_view.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    stationary_view.fovy(30).near(0.1).far(100);
    stationary_view.viewport(0, 0, fbWidth(), fbHeight());

    cubemap.createCubemap(cuberes, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    rbo.create(cuberes, cuberes);
    fbo.bind();
    fbo.attachCubemapFace(cubemap, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    fbo.attachRBO(rbo);
    fbo.unbind();
    printf("fbo status %s\n", fbo.statusString());

    cubeshader.compile(cubevert(), cubefrag());
    cubeshader.begin();
    cubeshader.uniform("tex0", 0);
    cubeshader.uniform("tex0_mix", 0.0);
    cubeshader.uniform("light_mix", 0.0f);
    cubeshader.end();

    cubesampleshader.compile(cubesamplevert(), cubesamplefrag());
    cubesampleshader.begin();
    cubesampleshader.uniform("cubemap", 0);
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
    g.polygonMode(Graphics::FILL);
    g.depthTesting(true);
    g.cullFace(true); // default front face is CCW, default cull face is BACK

    // bind cubemap fbo and capture 6 faces
    fbo.bind();

    g.shader(cubeshader);
    g.camera(cube_view);
    g.shader().uniform("omni_eyeSep", 0.01);
    g.shader().uniform("omni_radius", 1e10);

    for (int j = 0; j < 6; j++) {
      g.shader().uniform("C", get_cube_mat(j));
      fbo.attachCubemapFace(cubemap, GL_TEXTURE_CUBE_MAP_POSITIVE_X+j);
      g.clear(j / 5.0f, (5 - j) / 5.0f, 1.0f);
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

    g.camera(stationary_view);

    // draw cubemap
    g.shader(cubesampleshader);    
    g.texture(cubemap);
    g.draw(quad);

    // draw regular scene
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
  app.title("cubemap rendering test");
  app.displayMode(Window::DEFAULT_BUF);
  app.start(); // blocks
  return 0;
}