#include "al/core.hpp"
#include "al/util/al_Image.hpp"
#include <thread>
#include <atomic>

using namespace al;
using namespace std;

float const w = 256;
float const h = 256;

class MyApp : public EasyApp {
public:
  EasyFBO fbo;
  Mesh mesh;
  Mesh texRect; // to display fbo texture
  GLuint pbo;
  atomic<bool> done_saving {false};
  Image img;

  void onCreate() {
    fbo.init(w, h);

    addIcosahedron(mesh);

    view.pos(Vec3f(0, 0, 10));
    view.viewport(0, 0, w, h); // same as our fbo

    addTexRect(texRect, 0, 0, w, h);

    const int numBytes = fbWidth() * fbHeight() * 4; // rgb
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, numBytes, nullptr, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);


    // glReadBuffer specifies a color buffer as the source for subsequent glReadPixels
    // mode is initially GL_FRONT in single-buffered configurations
    // and GL_BACK in double-buffered configurations.
    // glReadBuffer(GL_BACK);

    int current_read_buffer;
    glGetIntegerv(GL_READ_BUFFER, &current_read_buffer);
    // cout << "GL_FRONT_LEFT: " << GL_FRONT_LEFT << endl;
    // cout << "GL_FRONT_RIGHT: " << GL_FRONT_RIGHT << endl;
    // cout << "GL_BACK_LEFT: " << GL_BACK_LEFT << endl;
    // cout << "GL_BACK_RIGHT: " << GL_BACK_RIGHT << endl;
    // cout << "GL_FRONT: " << GL_FRONT << endl;
    cout << "GL_BACK: " << GL_BACK << endl;
    // cout << "GL_LEFT: " << GL_LEFT << endl;
    // cout << "GL_COLOR_ATTACHMENT0: " << GL_COLOR_ATTACHMENT0 << endl;
    cout << "current read buffer? " << current_read_buffer << endl;

    img.resize(fbWidth(), fbHeight(), Image::RGBA);
  }

  void onAnimate(double dt) {
    if (frameCount() == 5) {
      double begin1 = msec();
      glReadPixels(0, 0, fbWidth(), fbHeight(), GL_RGBA, GL_UNSIGNED_BYTE, img.pixels());
      cout << "took1: " << msec() - begin1 << endl;

      double begin2 = msec();
      img.save("data/testing.png");
      cout << "took2: " << msec() - begin2 << endl;
    }

    if (frameCount() == 15) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
      double begin = msec();
      glReadPixels(0, 0, fbWidth(), fbHeight(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);
      cout << "took3: " << msec() - begin << endl;
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    if (frameCount() == 25) {
      const int numBytes = fbWidth() * fbHeight() * 4;

      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);

      double begin1 = msec();
      auto * ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
      cout << "took4: " << msec() - begin1 << endl;

      double begin2 = msec();
      memcpy(img.pixels(), ptr, numBytes);
      cout << "took5: " << msec() - begin2 << endl;

      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

      thread t {[&] () {
        img.save("data/testing2.png");
        done_saving = true;
      }};
      t.detach();

      // double begin3 = msec();
      // img.save("data/testing2.png");
      // cout << "took6: " << msec() - begin3 << endl;
    }

    if (done_saving) {
      cout << "done saving" << endl;
      done_saving = false;
    }
  }

  void onDraw() {
    g.framebuffer(fbo):
    g.clear(0, 1, 1);
    g.shader(color_shader);
    g.shader().uniform("col0", Color(1, 0, 0));
    g.camera(view);
    g.draw(mesh);

    g.framebuffer(FBO::DEFAULT):
    g.clear(0, 0, 1);
    g.shader(tex_shader);
    g.camera(Viewpoint::ORTHO_FOR_2D); // for 2D display of fbo texture
    fbo.tex().bind(0);
    g.shader().uniform("tex0", 0);
    g.draw(texRect);
    fbo.tex().unbind(0);
  }

};

int main() {
  MyApp app;
  app.dimensions(512, 512);
  app.start();
  return 0;
}