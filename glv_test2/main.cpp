#include "al/core.hpp"
#include "al/glv.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace al;
using namespace std;

class MyApp : public App {
public:
    ShaderProgram shader;
    Graphics g {*this};

    GlvGui glvGui{*this};
    SliderWithLabel labelSlider01{ "long test name 01", 10, 100 };
    SliderWithLabel labelSlider02{ "test02" };
    SliderWithLabel labelSlider03{ "" };
    ButtonWithLabel buttons {"my buttons", 13};
    NumberDialerWithLabel ndl {"dialer", 1, 1};

    void onCreate() {
        shader.compile(al_default_vert_shader(), al_default_frag_shader());

        glvGui.addSlider(labelSlider01);
        glvGui.addSlider(labelSlider02);
        glvGui.addSlider(labelSlider03);
        glvGui.addSlider("test1!!", 0, 0.001, 0);
        glvGui.addSlider("testing!!");
        glvGui.addSlider("testing!!"); // will not register since there's already a slider with same name

        glvGui.addButton(buttons);
        glvGui.addButton("buttons2", 3, 4);
        glvGui.addButton("buttons");
        glvGui.addButton("buttons3", 13);
        
        glvGui.addNumberDialer(ndl);
        glvGui.addNumberDialer("my dialer");
        glvGui.addNumberDialer("my dialer1", 2, 3);
        glvGui.addNumberDialer("my dialer2", 2, 3, 100, 0);
    }

    void onAnimate(double dt) {
        
    }

    void onDraw() {
        g.shader(shader);
        g.clearColor(0, 0, 0, 1);
        glvGui.draw(g);
    }

    virtual void onKeyDown(Keyboard const& k) override {
        if (k.key() == '1') {
            glvGui.removeNumberDialer("my dialer");
        }
        if (k.key() == '2') {
            glvGui.removeSlider("testing!!");
        }
        if (k.key() == '3') {
            glvGui.removeButton("buttons2");
        }
        if (k.key() == '4') {
            glvGui.removeNumberDialer(ndl);
        }
        if (k.key() == '5') {
            glvGui.removeSlider(labelSlider01);
        }
        if (k.key() == '6') {
            glvGui.removeButton(buttons);
        }
    }
};

int main() {
    MyApp app;
    app.dimensions(640, 480);
    app.title("glv test");
    app.start();
    return 0;
}