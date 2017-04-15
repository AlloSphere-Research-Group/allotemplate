#include "cubemap.hpp"
#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

int const cuberes {512};

class MyApp : public App {
public:
  NavInputControl nav;
  ShaderProgram shader;
  VAOMesh mesh, texquad;
  Texture cubesampletex;
  CubeRender cube_render;
  CubeSampler cube_sampler;
  Graphics g {*this};

  void onCreate() {
    append(nav.target(cube_render.view_));

    addIcosahedron(mesh);
    auto num_verts = mesh.vertices().size();
    for (int i = 0; i < num_verts; i++) {
      mesh.color(i / float(num_verts), (num_verts - i) / float(num_verts), 0.0);
    }
    mesh.update();

    addTexQuad(texquad);
    texquad.update();

    cube_render.init(cuberes);

    // we don't have warp/blend texture now
    // so generate dummy texture (equirectangular)
    int sampletex_width = 4 * cuberes;
    int sampletex_height = 2 * cuberes;
    cubesampletex.create2D(sampletex_width, sampletex_height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    vector<float> arr;
    arr.resize(sampletex_width * sampletex_height * 4);
    for (int i = 0; i < sampletex_width; i++) {
        float longi = i / float(sampletex_width) * 3.1415926535 * 2.0;
        for (int j = 0; j < sampletex_height; j++) {
            int idx = i + sampletex_width * j;
            float latti = (j / float(sampletex_height) - 0.5) * 3.1415926535;
            arr[4 * idx + 0] = cosf(longi) * cosf(latti);
            arr[4 * idx + 1] = sinf(latti);
            arr[4 * idx + 2] = sinf(longi) * cosf(latti);
            arr[4 * idx + 3] = 0.0f;
        }
    }
    cubesampletex.bind();
    cubesampletex.submit(arr.data()); // give raw pointer
    cubesampletex.update();
    cubesampletex.unbind();

    cube_sampler.init();
    cube_sampler.sampleTexture(cubesampletex);
    cube_sampler.cubemap(cube_render.cubemap_);
  }

  void onAnimate(double dt) {
      nav.step();
  }

  void onDraw() {
    g.polygonMode(Graphics::FILL);
    g.depthTesting(true);
    g.cullFace(true); // default front face is CCW, default cull face is BACK

    // bind cubemap fbo and capture 6 faces
    cube_render.begin(g);
    cube_render.set_eye(-1);
    for (int i = 0; i < 6; i++) {
      cube_render.set_face(i);
      g.clearColor(i / 5.0f, (5 - i) / 5.0f, 1.0f);
      g.clearDepth(1);
      g.pushMatrix();
      g.translate(sinf(sec()), 0, -10);
      g.rotate(sinf(2 * sec()), 0, 0, 1);
      g.rotate(sinf(3 * sec()), 0, 1, 0);
      g.scale(3, 2, 1);
      g.draw(mesh);
      g.popMatrix();
    }
    cube_render.end();

    // now sample cubemap and draw result to quad
    g.clearColor(0, 0, 0);
    g.clearDepth(1);

    cube_sampler.set_shader_and_texture(g);
    g.camera(Viewpoint::IDENTITY);
    g.draw(texquad); // fill viewport
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