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
  double delta_t;
  glv::GLV glv;
  glv::Slider sl1 {glv::Rect(150,150,300,30)};
  glv::Slider sl2 {glv::Rect(200,200,200,50)};
  float var = 1; // !!!

  glv::Sliders sliders{glv::Rect{200,80}, 1, numSliders};
  glv::Label labels[numSliders];

  glv::Slider slider{glv::Rect{100,20}};
  glv::Label label;

  glv::Placer p {glv, glv::Direction::E, glv::Place::TL, 10, 10};

  glv::View v{glv::Rect{100,100, 600,400}};
  glv::View v1{glv::Rect{10,10, 300,200}}, v2{glv::Rect{v1.right()+10,10, 100,200}};
  glv::View v11{glv::Rect{20,20, 80,100}}, v12{glv::Rect{80,80,100,80}};


void onCreate() {
    shader.compile(al_default_vert_shader(), al_default_frag_shader());
    
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
    sliders.colors().set(glv::StyleColor::SmokyGray);

    glv << sliders;
    sliders.setValue(0.2, 0);
    sliders.setValue(0.3, 1);
    sliders.setValue(0.4, 2);
    sliders.setValue(0.5, 3);

    slider.attach(ntSetLabel, glv::Update::Value, &label);
    p << slider << label;
    slider.setValue(0.5);

    glv << v;
    v << v1 << v2;
    v1 << v11 << v12;
    
    // Set properties of Views  
    glv::View* views[] = {&v, &v1, &v2, &v11, &v12};
    for(int i=0; i<5; ++i){
      views[i]->addHandler(glv::Event::MouseDrag, glv::Behavior::mouseMove);
      //views[i]->enable(KeepWithinParent);
    }
    
    // Disable some of the default View properties
    v.disable(glv::DrawBack);
    v2.disable(glv::DrawBorder);
    v12.disable(glv::FocusHighlight);
    
    // Set color styles
    glv.cloneStyle().colors().set(glv::StyleColor::WhiteOnBlack);
    v1.colors().set(glv::Color(0.2,0.4,1,0.8), 0.7);
  }

  void onAnimate(double dt) {
    delta_t = dt;
  }

  void onDraw() {
    g.shader(shader);
    g.clearColor(0.5, 0.55, 0.6, 1.0);
    al_draw_glv(glv, g, delta_t);
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
    auto b = m.button();
    auto btn = glv::Mouse::Left;
    switch (b) {
      case al::Mouse::LEFT: break;
      case al::Mouse::MIDDLE: btn = glv::Mouse::Middle; break;
      case al::Mouse::RIGHT: btn = glv::Mouse::Right; break;
    }
    glv.setMouseDown(x, y, btn, 0);
    glv.setMousePos(x, y, x, y);
    glv.propagateEvent();
  }

  void onMouseUp(Mouse const& m) {
    float x = m.x();
    float y = m.y();
    auto b = m.button();
    auto btn = glv::Mouse::Left;
    switch (b) {
      case al::Mouse::LEFT: break;
      case al::Mouse::MIDDLE: btn = glv::Mouse::Middle; break;
      case al::Mouse::RIGHT: btn = glv::Mouse::Right; break;
    }
    glv.setMouseUp(x, y, btn, 0);
    glv.setMousePos(x, y, x, y);
    glv.propagateEvent();
  }

  void onMouseDrag(Mouse const& m) {
    float x = m.x();
    float y = m.y();
    glv.setMouseMotion(x, y, glv::Event::MouseDrag);
    glv.setMousePos(x, y, x, y);
    glv.propagateEvent();
  }

  void onMouseMove(Mouse const& m) {
    
  }

  void onResize(int w, int h) {
    
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