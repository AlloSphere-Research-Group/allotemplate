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

    Graphics g {this};
    ShaderProgram shader;
    EasyVAO vao;
    Mesh mesh;
    // VAOMesh mesh;
    vec3 pos[3];
    bool flag {false};
    VAOMesh m0;
    VAOMesh m1;

    void onCreate()
    {
        shader.compile(al_color_vert_shader(), al_color_frag_shader());

        pos[0] = vec3(50, 50, 0);
        pos[1] = vec3(200, 50, 0);
        pos[2] = vec3(50, 100, 0);

        // vao.primitive(GL_LINE_STRIP);
        vao.primitive(GL_TRIANGLES);
        vao.updatePosition(pos, 3);

        // mesh.primitive(Mesh::TRIANGLES);
        mesh.primitive(Mesh::LINE_STRIP);
        mesh.reset();
        mesh.vertex(100, 50, 0);
        mesh.vertex(250, 50, 0);
        mesh.vertex(100, 100, 0);
        // mesh.update();

        m0.primitive(Mesh::TRIANGLES);
        m0.vertex(10, 10, 0);
        m0.vertex(200, 50, 0);
        m0.vertex(50, 100, 0);
        m0.update();
        m1 = m0;
        m0.vertices()[0] = Vec3f(50, 50, 0);
        m0.update();
        cout << m0.vertices()[0].x << endl;
        cout << m1.vertices()[0].x << endl;
    }

    void onAnimate(double dt)
    {

    }

    void onDraw()
    {
        g.clearColor(1, 1, 1);
        g.shader(shader);
        g.camera(Viewpoint::ORTHO_FOR_2D);
        g.shader().uniform("col0", Color(1, 0.5, 0.2));
        g.cullFace(false);
        if (flag) {
            g.draw(vao);
            // g.draw(m0);
            // vao.draw();
        }
        else {
            // g.draw(m0);
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