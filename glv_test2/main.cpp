#include "al/core.hpp"

#include "al/glv/glv.h"
#include "glv_draw_al.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace al;
using namespace std;

class GLVEventHandler : public WindowEventHandler {
public:
	glv::GLV* mGlvPtr;

	GLVEventHandler(glv::GLV& g) : mGlvPtr{ &g } {}

	virtual bool mouseDown(const Mouse& m) {
		float relx = m.x();
		float rely = m.y();
		auto btn = glv::Mouse::Left;
		switch (m.button()) {
			case al::Mouse::MIDDLE: btn = glv::Mouse::Middle; break;
			case al::Mouse::RIGHT: btn = glv::Mouse::Right; break;
		}
		mGlvPtr->setMouseDown(relx, rely, btn, 0);
		mGlvPtr->setMousePos(int(m.x()), int(m.y()), relx, rely);
		bool consumed = mGlvPtr->propagateEvent();
		return !consumed;
	}

	virtual bool mouseDrag(const Mouse& m) {
		float x = m.x();
		float y = m.y();
		float relx = x;
		float rely = y;
		mGlvPtr->setMouseMotion(relx, rely, glv::Event::MouseDrag);
		mGlvPtr->setMousePos(int(x), int(y), relx, rely);
		bool consumed = mGlvPtr->propagateEvent();
		return !consumed;
	}

	virtual bool mouseUp(const Mouse& m) {
		float relx = m.x();
		float rely = m.y();
		auto btn = glv::Mouse::Left;
		switch (m.button()) {
			case al::Mouse::MIDDLE: btn = glv::Mouse::Middle; break;
			case al::Mouse::RIGHT: btn = glv::Mouse::Right; break;
		}
		mGlvPtr->setMouseUp(relx, rely, btn, 0);
		mGlvPtr->setMousePos(int(m.x()), int(m.y()), relx, rely);
		bool consumed = mGlvPtr->propagateEvent();
		return !consumed;
	}

	virtual bool resize(int dw, int dh) {
		mGlvPtr->extent(dw, dh);
		mGlvPtr->broadcastEvent(glv::Event::WindowResize);
		return true;
	}
};

class SliderMinMax : public glv::Slider {
public:
	SliderMinMax(double min, double max, double v)
		: glv::Slider(), minimum(min), maximum(max)
	{
		setValue((v - minimum) / (maximum - minimum));
	}

	double getCalcedValue() {
		return getValue() * (maximum - minimum) + minimum;
	}

	double minimum, maximum;
};

class SliderWithLabel {
public:
	static void updateLabel(const glv::Notification& n) {
		glv::Label* l = n.receiver<glv::Label>();
		SliderMinMax* s = n.sender<SliderMinMax>();
		l->setValue(glv::toString(s->getCalcedValue()));
	}

	SliderWithLabel(std::string n): SliderWithLabel{ n, 0, 1, 0.5 } {}
	SliderWithLabel(std::string n, double min, double max)
		: SliderWithLabel{n, min, max, 0.5 * (max + min)} {}

	SliderWithLabel(std::string n, double min, double max, double v)
		: label{ n }, slider{ min, max, v },
		valueLabel{ glv::toString(v), glv::Place::CL, 5, 0 }
	{
		slider.attach(SliderWithLabel::updateLabel, glv::Update::Value, &valueLabel);
		slider << valueLabel;
	}

	void value(double val) {
		slider.setValue(val);
	}

	double value() {
		return  slider.getCalcedValue();
	}

	SliderMinMax slider;
	glv::Label label;
	glv::Label valueLabel;
};

class GlvGui {
public:
	static int const max_sliders = 64; // who makes more than 64 sliders!?

	glv::GLV mGlv;
	glv::Table mTable{"><", 5, 5};
	GLVEventHandler eventHandler{ mGlv };
	Window* mWindowPtr;
	std::vector<SliderWithLabel> sliders;
	std::unordered_map<std::string, int> slider_index_map;

	GlvGui(Window& window, bool blackLetters = false) : mWindowPtr{ &window } {
		mGlv.extent(mWindowPtr->width(), mWindowPtr->height());
		mGlv.broadcastEvent(glv::Event::WindowResize);
		if (blackLetters) {
			mGlv.colors().set(glv::StyleColor::Gray);
		}
		else {
			mGlv.colors().set(glv::StyleColor::SmokyGray);
		}
		mWindowPtr->append(eventHandler);
		mGlv << mTable;
		sliders.reserve(max_sliders); // make sure vector does not re-allocate >> preserves pointer validness
	}

	glv::GLV& glv() { return mGlv; }

	void addSlider(SliderWithLabel& slider) {
		mTable << slider.slider;
		mTable << slider.label;
		mTable.arrange();
	}

	void addSlider(std::string name, double min, double max, double val) {
		// map easily invalidates iterators. so let's store objec in reserved vector,
		// and later find specific object with index stored in map with name string
		auto search = slider_index_map.find(name);
		if (search != slider_index_map.end()) {
			std::cout << "slider with name \"" << name << "\" already exists" << std::endl;
			return;
		}
		sliders.emplace_back(name, min, max, val);
		int idx = sliders.size() - 1;
		slider_index_map.emplace(name, idx);
		addSlider(sliders.back());
	}
	void addSlider(std::string name) { addSlider(name, 0, 1, 0.5); }
	void addSlider(std::string name, double min, double max) { addSlider(name, min, max, 0.5 * (min + max)); }

	double sliderValue(std::string name) {
		auto search = slider_index_map.find(name);
		if (search == slider_index_map.end()) {
			std::cout << "no slider with name \"" << name << "\" exists" << std::endl;
			return 0;
		}
		return sliders[search->second].value();
	}

	void draw(Graphics& g) {
		al_draw_glv(mGlv, g);
	}
};

class MyApp : public App {
public:
  ShaderProgram shader;
  Graphics g {*this};

  GlvGui glvGui{ *this };
  SliderWithLabel labelSlider01{ "long test name 01", 10, 100 };
  SliderWithLabel labelSlider02{ "test02" };
  SliderWithLabel labelSlider03{ "" };

void onCreate() {
    shader.compile(al_default_vert_shader(), al_default_frag_shader());
    
	glvGui.addSlider(labelSlider01);
	glvGui.addSlider(labelSlider02);
	glvGui.addSlider(labelSlider03);
	glvGui.addSlider("test1!!", 0, 0.001, 0);
	glvGui.addSlider("test2!!");
	glvGui.addSlider("testing!!");
	glvGui.addSlider("testing!!"); // will not register since there's already a slider with same name
  }

  void onAnimate(double dt) {
	  std::cout << glvGui.sliderValue("test1!!") << std::endl;
  }

  void onDraw() {
    g.shader(shader);
    g.clearColor(0.1, 0.1, 0.1, 1);
	glvGui.draw(g);
  }
};

int main() {
  MyApp app;
  app.dimensions(640, 480);
  app.title("glv test");
  app.start();
  return 0;
}