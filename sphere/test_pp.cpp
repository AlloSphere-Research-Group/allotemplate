#include "perprojection.hpp"
#include "sphere_utils.hpp"

#include "al/core.hpp"

#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

// int const cuberes {512};

class MyApp : public App {
public:
  NavInputControl nav;
  VAOMesh mesh;
  Texture cubesampletex;
  PerProjectionRender pp_render;
  Graphics g {this};

  void onInit()
  {
      if (sphere::is_renderer()) {
          int width, height;
          sphere::get_fullscreen_dimension(&width, &height);
          if (width != 0 && height != 0) {
              // cout << "width: " << width << " height: " << height << endl;
              dimensions(0, 0, width, height);
              decorated(false);
          }
          else {
              cout << "[!] calculated width and/or height are/is zero!" << endl;
          }
      }
  }

  void onCreate() {
    // Load the calibration data first
    pp_render.load_calibration_data(
      sphere::config_directory("data").c_str(),
      sphere::renderer_hostname("config").c_str()
    );
    // pp_render.load_calibration_data("../../calibration_data", "gr03");
    pp_render.init(1024);

    append(nav.target(pp_render.view_));

    // // we don't have warp/blend texture now
    // // so generate dummy sampling texture (equirectangular)
    // int sampletex_width = 4 * cuberes;
    // int sampletex_height = 2 * cuberes;
    // cubesampletex.create2D(sampletex_width, sampletex_height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    // vector<float> arr;
    // arr.resize(sampletex_width * sampletex_height * 4);
    // for (int i = 0; i < sampletex_width; i++) {
    //     float longi = i / float(sampletex_width) * 3.1415926535 * 2.0;
    //     for (int j = 0; j < sampletex_height; j++) {
    //         int idx = i + sampletex_width * j;
    //         float latti = (j / float(sampletex_height) - 0.5) * 3.1415926535;
    //         arr[4 * idx + 0] = cosf(longi) * cosf(latti);
    //         arr[4 * idx + 1] = sinf(latti);
    //         arr[4 * idx + 2] = sinf(longi) * cosf(latti);
    //         arr[4 * idx + 3] = 0.0f;
    //     }
    // }
    // cubesampletex.submit(arr.data()); // give raw pointer

    // pp_sampler.init();
    // pp_sampler.sampleTexture(cubesampletex);
    // pp_sampler.cubemap(cube_render.cubemap_);

    // // user code
    addIcosahedron(mesh);
    auto num_verts = mesh.vertices().size();
    for (int i = 0; i < num_verts; i++) {
      mesh.color(i / float(num_verts), (num_verts - i) / float(num_verts), 0.0);
    }
    mesh.update();
  }

  void onAnimate(double dt) {
      nav.step();
  }

  void onDraw() {
    g.polygonMode(Graphics::FILL);
    g.depthTesting(false);
    g.cullFace(true); // default front face is CCW, default cull face is BACK

    // // bind cubemap fbo and capture 6 faces
    pp_render.begin(g);
    for (int eye = 0; eye < pp_render.num_eyes(); eye += 1) { // 2 for stereo (TODO!)
      pp_render.set_eye(eye);
      for (int i = 0; i < pp_render.num_projections(); i++) {
        pp_render.set_projection(i);

        // user code
        g.clearColor(i / 5.0f, (5 - i) / 5.0f, 1.0f);
        g.clearDepth(1);
        for(int aa = -5; aa <= 5; aa++)
        for(int bb = -5; bb <= 5; bb++)
        for(int cc = -5; cc <= 5; cc++)  {
          if(aa == 0 && bb == 0 && cc == 0) continue;
        g.pushMatrix();
        g.translate(aa * 2, bb * 2, cc * 2);
        g.rotate(sinf(2 * sec()), 0, 0, 1);
        g.rotate(sinf(3 * sec()), 0, 1, 0);
        g.scale(0.1, 0.1, 0.1);
        g.draw(mesh);
        g.popMatrix();
        }
        // end of user code

      }
    }
    pp_render.end();

    // now sample cubemap and draw result to quad
    g.clearColor(0, 0, 0);
    g.clearDepth(1);
    pp_render.composite(g);
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