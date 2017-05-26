#include "al/core.hpp"

#include "al/glv/glv.h"
#include "glv_draw_al.hpp"

#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

const int numSliders = 4;

void ntSetLabel(const glv::Notification& n){
  glv::Label * l = n.receiver<glv::Label>();
  glv::Sliders& s = *n.sender<glv::Sliders>();
  
  // Get currently selected slider
  int idx = s.selected();
  
  // Set string of corresponding label to slider value
  l[idx].setValue(glv::toString(s.getValue(idx)));
}

class MyApp : public App {
public:
  ShaderProgram shader;
  Graphics g {*this};

  glv::GLV glv;
  glv::Slider sl1 {glv::Rect(150,150,300,30)};
  glv::Slider sl2 {glv::Rect(200,200,200,50)};
  float var = 1;

  glv::Sliders sliders{glv::Rect{200,80}, 1, numSliders};
  glv::Label labels[numSliders];

  glv::Slider slider{glv::Rect{100,20}};
  glv::Label label;

  glv::View v{glv::Rect{100,100, 600,400}};
  glv::View v1{glv::Rect{10,10, 300,200}}, v2{glv::Rect{v1.right()+10,10, 100,200}};
  glv::View v11{glv::Rect{20,20, 80,100}}, v12{glv::Rect{80,80,100,80}};

void onCreate() {
    shader.compile(al_default_vert_shader(), al_default_frag_shader());
    
    glv.extent(width(), height());
    glv.broadcastEvent(glv::Event::WindowResize);
    glv.colors().set(glv::StyleColor::SmokyGray);
    glv::graphicsHolder().set(g); // register graphics object

    sl1.attachVariable(var);
    sl2.attachVariable(var);

    glv << sl1 << sl2;

    for(int i=0; i<numSliders; ++i){
      labels[i].pos(2,4);
      // Set anchor factor so labels lie on top of each slider
      labels[i].anchor(0, float(i)/numSliders);

      sliders << labels[i];
    }

    sliders.attach(ntSetLabel, glv::Update::Value, labels);

    glv << sliders;
    sliders.setValue(0.2, 0);
    sliders.setValue(0.3, 1);
    sliders.setValue(0.4, 2);
    sliders.setValue(0.5, 3);

    slider.attach(ntSetLabel, glv::Update::Value, &label);
    slider.setValue(0.5);

    glv << v;
    v << v1 << v2;
    v1 << v11 << v12;
    
    // Set properties of Views  
    glv::View* views[] = {&v, &v1, &v2, &v11, &v12};
    for(int i=0; i<5; ++i){
      views[i]->addHandler(glv::Event::MouseDrag, glv::Behavior::mouseMove);
    }
    
    // Disable some of the default View properties
    v.disable(glv::DrawBack);
    v2.disable(glv::DrawBorder);
    v12.disable(glv::FocusHighlight);

    
    float range[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
    cout << "aliased line [" << range[0] << ":" << range[1] << "]" << endl;
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, range);
    cout << "smooth line [" << range[0] << ":" << range[1] << "]" << endl;
    float val;
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_GRANULARITY, &val);
    cout << "smooth line granularity [" << val << "]" << endl;
    glGetFloatv(GL_POINT_SIZE_RANGE, range);
    cout << "point size [" << range[0] << ":" << range[1] << "]" << endl;

    // glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    // glEnable(GL_LINE_SMOOTH);
    // glLineWidth(1);
  }

  void onAnimate(double dt) {

  }

  void onDraw() {
    g.shader(shader);
    g.clearColor(0.7, 0.6, 0.5, 1);

    g.camera(Viewpoint::ORTHO_FOR_2D);
    g.pushMatrix();
    g.loadIdentity();
    g.translate(0, height());
    g.scale(1, -1);
    g.blending(false);
    g.depthTesting(false);
    g.textureMix(0, 0, 0, 0);
    g.uniformColorMix(1);
    glv::color(1, 0, 0, 1);
    glv::rectangle(100, 100, 200, 200);
    glv::text("this is\n\tworking", 200, 50, 40);
    g.popMatrix();

    al_draw_glv(glv, g);
  }

  void onKeyDown(Keyboard const& k) {
    glv.setKeyModifiers(
      k.shift(),
      k.alt(),
      k.ctrl(),
      k.caps(),
      k.meta()
    );
  }

  void onKeyUp(Keyboard const& k) {
    glv.setKeyModifiers(
      k.shift(),
      k.alt(),
      k.ctrl(),
      k.caps(),
      k.meta()
    );
  }

  void onMouseDown(Mouse const& m) {
    float x = m.x();
    float y = m.y();
    float relx = x;
    float rely = y;
    auto b = m.button();
    auto btn = glv::Mouse::Left;
    switch (b) {
      case al::Mouse::LEFT: break;
      case al::Mouse::MIDDLE: btn = glv::Mouse::Middle; break;
      case al::Mouse::RIGHT: btn = glv::Mouse::Right; break;
    }
    glv.setMouseDown(relx, rely, btn, 0);
    glv.setMousePos(int(x), int(y), relx, rely);
    glv.propagateEvent();
  }

  void onMouseUp(Mouse const& m) {
    float x = m.x();
    float y = m.y();
    float relx = x;
    float rely = y;
    auto b = m.button();
    auto btn = glv::Mouse::Left;
    switch (b) {
      case al::Mouse::LEFT: break;
      case al::Mouse::MIDDLE: btn = glv::Mouse::Middle; break;
      case al::Mouse::RIGHT: btn = glv::Mouse::Right; break;
    }
    glv.setMouseUp(relx, rely, btn, 0);
    glv.setMousePos(int(x), int(y), relx, rely);
    glv.propagateEvent();
  }

  void onMouseDrag(Mouse const& m) {
    float x = m.x();
    float y = m.y();
    float relx = x;
    float rely = y;
    glv.setMouseMotion(relx, rely, glv::Event::MouseDrag);
    glv.setMousePos(int(x), int(y), relx, rely);
    glv.propagateEvent();
  }

  void onMouseMove(Mouse const& m) {
      //float x = m.x();
      //float y = m.y();
      //float relx = x;
      //float rely = y;
      //glv.setMouseMotion(relx, rely, glv::Event::MouseMove);
      //glv.setMousePos(int(x), int(y), relx, rely);
      //glv.propagateEvent();
  }

  void onResize(int w, int h) {
      glv.extent(w, h);
      glv.broadcastEvent(glv::Event::WindowResize);
  }

  void onExit() {
    cout << "onExit" << endl;
  }

};

int main() {
  MyApp app;
  app.dimensions(640, 480);
  app.title("glv test");
  app.start();
  return 0;
}