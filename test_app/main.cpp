#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

class MyApp : public App {
public:
  ShaderProgram shader;
  VAOMesh mesh;
  Graphics g;
  osc::Recv server { 16447, "", 0.05 };
  Texture texture;
  FBO fbo;
  Texture color_attachment;
  RBO depth_attachment;
  Viewpoint vp;

  void onInit() {
    cout << "after glfw init, before window creation" << endl;
    // can do things such as setting window dimenstion from monitor data (with glfw functions)
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    dimensions(mode->width / 4, mode->height / 2);
  }
  
  void onCreate() {
    string const vert_source = R"(
      #version 330

      uniform mat4 m;

      layout (location = 0) in vec4 position;
      layout (location = 1) in vec4 color;
      layout (location = 2) in vec2 texcoord;

      out vec4 _color;
        out vec2 _texcoord;

      void main() {
        gl_Position = m * position;
        _color = color;
_texcoord = texcoord;
      }
    )";

    string const frag_source = R"(
      #version 330

      uniform sampler2D tex0;
    uniform float t;
      in vec4 _color;
        in vec2 _texcoord;

      out vec4 frag_color;

      void main() {
        vec4 texval = texture(tex0, _texcoord);
        frag_color = mix(_color, texval, t);
      }
    )";

    shader.compile(vert_source, frag_source);
    shader.listParams();
    shader.begin();
    shader.uniform("tex0", 0);
    shader.uniform("t", 0.5);
    shader.end();

    int w = 16;
    int h = 16;
    int internal = GL_RGBA8; // GL_RGBA16/32F/16F,
                             // GL_DEPTH_COMPONENT32F/24/16, ...
    int format = GL_RGBA; // GL_RGB, GL_DEPTH_COMPONENT, ...
    int type = GL_FLOAT; // GL_UNSIGNED_BYTE (what data type will we give?)
    texture.create2D(w, h, internal, format, type);

    array<float, 16 * 16 * 4> arr;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i + 16 * j;
            arr[4 * idx + 0] = i / 15.0f;
            arr[4 * idx + 1] = j / 15.0f;
            arr[4 * idx + 2] = 1.0f;
            arr[4 * idx + 3] = 1.0f;
        }
    }
    texture.bind();
    texture.submit(arr.data()); // give raw pointer
    texture.mipmap(true); // turn on only if needed
    texture.update();
    texture.unbind();

    meshA();

    color_attachment.create2D(128, 128);
    depth_attachment.create(128, 128);
    fbo.attachTexture2D(color_attachment);
    //fbo.attachTexture2D(color_attachment, GL_COLOR_ATTACHMENT0); // same as above
    fbo.attachRBO(depth_attachment);
    printf("fbo status %s\n", fbo.statusString());

    fbo.begin();
    g.clear(0, 1, 0, 1); // later EasyFBO will have its own al::Graphics
    fbo.end();

    g.setClearColor(0, 1, 1);
    // g.viewport(0, 0, fbWidth(), fbHeight()); // glViewport works on framebuffer size

    server.handler(*this);
    server.start();

    vp.transform()
      .pos(Vec3f(0, 0, 10))
      .quat(Quatf::getBillboardRotation(
        Vec3f(0, 0, -1), // forward
        Vec3f(0, 1, 0) // up
      )
    );
    vp.viewport().set(0, 0, fbWidth(), fbHeight());
    vp.lens().fovy(30).near(0.1).far(100);

    g.viewport(vp.viewport());
  }

  void onAnimate(double dt) {
    vp.transform().pos(Vec3f(sin(sec()), 0, 10));
  }

  void onDraw() {
    g.clear();

    // float w = width();
    // float h = height();

    // simple projection matrix for now
    // auto proj = Matrix4f::scaling(h / w, 1.0f, 1.0f);
    auto proj = vp.projMatrix();
    auto view = vp.viewMatrix();
    // Matrix4f mat = Matrix4f::rotate(sec(), 0, 0, 1);
    // mat = Matrix4f::scaling(h / w, 1.0f, 1.0f) * mat;
    
    shader.begin();
    shader.uniform("t", sin(sec()));

    texture.bind();

    g.pushMatrix();
    g.rotate(sec(), 0, 0, 1);
    g.translate(-0.5, 0, 0);
    shader.uniform("m", proj * view * g.modelMatrix());
    mesh.draw();
    g.popMatrix();

    texture.unbind();

    color_attachment.bind();

    g.pushMatrix();
    g.rotate(2 * sec(), 0, 0, 1);
    g.translate(0.5, 0, 0);
    shader.uniform("m", proj * view * g.modelMatrix());
    mesh.draw();
    g.popMatrix();

    color_attachment.unbind();

    shader.end();
  }

  void onKeyDown(Keyboard const& k) {
      if (k.key() == '1') {
          meshA();
          return;
      }
      if (k.key() == '2') {
          meshB();
          return;
      }
  }

  void onMessage(osc::Message& m) {
    if (m.addressPattern() == "/test") {
        // Extract the data out of the packet
        std::string str;
        m >> str;

        // Print out the extracted packet data
        std::cout << "SERVER: recv " << str << endl;
    }
  }

  void onSound(AudioIOData& io) {
    static double phase {0};
    // Set the base frequency to 55 Hz
    double freq = 55/io.framesPerSecond();

    while(io()){
      // Update the oscillators' phase
      phase += freq;
      if(phase > 1) phase -= 1;

      // Generate two sine waves at the 5th and 4th harmonics
      float out1 = cos(5*phase * 2*M_PI);
      float out2 = sin(4*phase * 2*M_PI);

      // Send scaled waveforms to output...
      io.out(0) = out1*0.2;
      io.out(1) = out2*0.4;
    }
  }

  void onExit() {
    cout << "onExit" << endl;
  }

  void meshA() {
      mesh.reset();
      mesh.primitive(TRIANGLES);
      mesh.vertex(-0.5, -0.5, 0);
      mesh.color(1.0, 0.0, 0.0);
      mesh.texCoord(0.0, 0.0);

      mesh.vertex(0.5, -0.5, 0);
      mesh.color(0.0, 1.0, 0.0);
      mesh.texCoord(1.0, 0.0);

      mesh.vertex(-0.5, 0.5, 0);
      mesh.color(0.0, 0.0, 1.0);
      mesh.texCoord(0.0, 1.0);

      mesh.vertex(-0.5, 0.5, 0);
      mesh.color(0.0, 0.0, 1.0);
      mesh.texCoord(0.0, 1.0);

      mesh.vertex(0.5, -0.5, 0);
      mesh.color(0.0, 1.0, 0.0);
      mesh.texCoord(1.0, 0.0);

      mesh.vertex(0.5, 0.5, 0);
      mesh.color(0.0, 1.0, 1.0);
      mesh.texCoord(1.0, 1.0);
      mesh.update(); // send to gpu buffers
  }

  void meshB() {
      mesh.reset();
      mesh.primitive(TRIANGLE_STRIP);
      mesh.vertex(-0.5, -0.5, 0);
      mesh.color(0.0, 0.0, 0.0);
      mesh.texCoord(0.0, 0.0);

      mesh.vertex(0.5, -0.5, 0);
      mesh.color(1.0, 0.0, 0.0);
      mesh.texCoord(1.0, 0.0);

      mesh.vertex(0.5, 0.5, 0);
      mesh.color(1.0, 1.0, 0.0);
      mesh.texCoord(1.0, 1.0);

      mesh.vertex(-0.5, 0.5, 0);
      mesh.color(0.0, 1.0, 0.0);
      mesh.texCoord(0.0, 1.0);

      mesh.index(0);
      mesh.index(1);
      mesh.index(3);
      mesh.index(2);
      mesh.update();
  }
};

int main() {
  MyApp app;
  app.initAudio();
  app.dimensions(640, 480);
  app.title("app test");
  app.fps(60);
  app.decorated(true);
  app.displayMode(Window::DEFAULT_BUF);
  app.start(); // blocks
  return 0;
}