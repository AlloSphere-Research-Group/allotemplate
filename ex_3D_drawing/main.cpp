#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

// example of 3D drawing with barebone app
// a lot of elements will be hidden later

class MyApp : public App {
public:
  Viewpoint viewpoint;
  NavInputControl nav;
  ShaderProgram shader;
  Texture texture;
  VAOMesh mesh, mesh2;

  void onCreate() {
    // set nav input event handler so that we can navigate with qwe/asd/zxc
    // also set viewpoint to be the pose that nav manipulates
    append(nav.target(viewpoint));
    
    // shader module has global function for default style shader
    // currently lighting is not supported
    shader.compile(al_default_vert_shader(), al_default_frag_shader());
    // shader.listParams();

    texture.create2D(
      16, 16, // width and height
      GL_RGBA8, // internal storage
      GL_RGBA, // format of data we give
      GL_FLOAT // type of data we give
    );
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

    addIcosahedron(mesh2);
    auto num_verts = mesh2.vertices().size();
    for (int i = 0; i < num_verts; i++) {
      mesh2.color(i / float(num_verts), (num_verts - i) / float(num_verts), 0.0);
    }
    mesh2.update(); // should update after add*** functions

    // viewpoint is: (eye position) + (projection lens) + (viewport area)
    viewpoint.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    viewpoint.fovy(30).near(0.1).far(100);

    // viewport takes framebuffersize, not window size
    // ex: a 640x480 window in MacBook Retina display
    // -> the framebuffer of window is 1280x960
    // but the window size is still 640x480
    viewpoint.viewport(0, 0, fbWidth(), fbHeight()); 
  }

  void onAnimate(double dt) {
    // update nav in frame unit
    nav.step();
  }

  void onDraw() {
    g.depthTesting(true);
    g.cullFace(true); // default front face is CCW, default cull face is BACK

    g.clearColor(0, 1, 1);
    g.clearDepth(1);

    // g.polygonMode(Graphics::POINT);
    // g.pointSize(10);
    // g.polygonMode(Graphics::LINE);
    g.polygonMode(Graphics::FILL);

    g.shader(shader); // note that we don't need to 'end()' for 'unbind()'
    g.camera(viewpoint);
    g.texture(texture); // again, note that we don't need to 'end()' for 'unbind()'

    // Graphics::shader() returns the shader currently used
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

  void onResize(int w, int h) {
    // update viewport
    viewpoint.viewport(0, 0, fbWidth(), fbHeight());
  }
};

int main() {
  // things we declare before app starts
  MyApp app;
  app.dimensions(640, 480);
  app.title("app test");
  app.fps(60);
  app.decorated(true);
  app.displayMode(Window::DEFAULT_BUF);
  app.start(); // blocks
  return 0;
}