#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

// any custom data class
class vec3
{
public:
    float x;
    float y;
    float z;
    vec3() : x(0), y(0), z(0) {}
    vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

class MyApp : public App
{
public:

    Graphics g {*this};
    ShaderProgram shader;
    EasyVAO vao;
    VAOMesh mesh;
    vec3 pos[3];
    bool flag {false};

    void onCreate()
    {
        shader.compile(al_default_vert_shader(), al_default_frag_shader());

        pos[0] = vec3(50, 50, 0);
        pos[1] = vec3(200, 50, 0);
        pos[2] = vec3(50, 100, 0);

        vao.primitive(GL_LINE_STRIP);
        vao.updatePosition(pos, 3);

        mesh.primitive(Mesh::TRIANGLES);
        mesh.reset();
        mesh.vertex(200, 50, 0);
        mesh.vertex(350, 50, 0);
        mesh.vertex(200, 100, 0);
        mesh.update();
    }

    void onAnimate(double dt)
    {

    }

    void onDraw()
    {
        g.clearColor(1, 1, 1);
        g.shader(shader);
        g.camera(Viewpoint::ORTHO_FOR_2D);
        g.uniformColor(1, 0.5, 0.2);
        g.uniformColorMix(1);
        g.update();
        if (flag) {
            g.draw(vao);
            // vao.draw();
        }
        else {
            g.draw(mesh);
            // mesh.draw();
        }
    }

    void onKeyDown(Keyboard const& k)
    {
        if (k.key() == ' ')
        {
            flag = !flag;
            cout << "flag: " << flag << endl;
        }
    }
};

int main() {
    MyApp app;
    app.start();
    return 0;
}