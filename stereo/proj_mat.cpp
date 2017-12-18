#include "al/core.hpp"
#include <iostream>
#include <cmath>
#include <string>

using namespace al;
using namespace std;

const string vert = R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
uniform float eye_sep;
uniform float foc_len;
uniform float is_omni;

layout (location = 0) in vec3 position;

vec4 flat_stereo(vec4 v, float e, float f) {
    float l = sqrt((v.x - e) * (v.x - e) + v.y * v.y + v.z * v.z);
    float z = abs(v.z);
    float t = f * (v.x - e) / z;
    v.x = z * (e + t) / f;
    v.xyz = normalize(v.xyz);
    v.xyz *= l;
    return v;
}

vec4 omni_stereo(vec4 v, float e, float r) {
    vec3 OE = vec3(-v.z, 0.0, v.x); // eye direction, orthogonal to vertex vector
    OE = normalize(OE);             // but preserving +y up-vector
    OE *= e;               // set mag to eye separation
    vec3 EV = v.xyz - OE;      // eye to vertex
    float ev = length(EV); // save length
    EV /= ev;              // normalize

    // coefs for polynomial t^2 + 2bt + c = 0
    // derived from cosine law r^2 = t^2 + e^2 + 2tecos(theta)
    // where theta is angle between OE and EV
    // t is distance to sphere surface from eye
    float b = -dot(OE, EV);         // multiply -1 to dot product because
                                     // OE needs to be flipped in direction
    float c = e * e - r * r;
    float t = -b + sqrt(b * b - c);// quadratic formula

    v.xyz = OE + t * EV;           // direction from origin to sphere surface
    v.xyz = ev * normalize(v.xyz); // normalize and set mag to eye-to-v distance
    return v; 
}

void main() {
    vec4 vert_eye = MV * vec4(position, 1.0);
    vec4 displaced = flat_stereo(vert_eye, eye_sep, foc_len);
    if (is_omni > 0.5) {
        displaced = omni_stereo(vert_eye, eye_sep, foc_len);
    }
    gl_Position = P * displaced;
}
)";

const string frag = R"(
#version 330
uniform vec4 color;
out vec4 frag_color;
void main() {
  frag_color = color;
}
)";

struct pm : App
{
    const unsigned char use_shader_displacement = 1 << 0;
    const unsigned char is_right_eye            = 1 << 1;
    const unsigned char is_stereo               = 1 << 2;
    const unsigned char flicker                 = 1 << 3;
    const unsigned char lighting                = 1 << 4;
    const unsigned char coloring                = 1 << 5;
    const unsigned char is_omni                 = 1 << 6;
    unsigned char flags = 0;

    ShaderProgram shader;
    Light light;
    VAOMesh m;
    // Mesh m;

    void onCreate() override
    {
        addSphere(m, 0.1);
        m.generateNormals();
        m.update();
        light.pos(0, 0, 0);
        light.ambient({0.2});
        graphics().light(light);
        lens().eyeSep(0.1);
        lens().focalLength(3);
        lens().fovy(30);
        shader.compile(vert, frag);
    }

    void onDraw(Graphics& g) override
    {
        static unsigned int frame_num = ~0;
        frame_num += 1;

        g.clear(0.1);
        g.depthTesting(true);
        // g.polygonMode(Graphics::LINE);

        float ar = width() / float(height());
        float l = lens().focalLength();
        float e = lens().eyeSep();
        float n = lens().near();
        float f = lens().far();
        float y = lens().fovy();
        
        cout << '\r' << fps() << '\t' << rnd::uniform() << flush;

        if (flags & use_shader_displacement) {
            float eye_dir = 0.0f;
            if (flags & flicker) {
                eye_dir = (frame_num % 2)? 1.0f : -1.0f;
                
            }
            else if (flags & is_stereo) {
                eye_dir = (flags & is_right_eye)? 1.0f : -1.0f;
            }

            g.shader(shader);
            g.shader().uniform("eye_sep", e * eye_dir);
            g.shader().uniform("foc_len", l);

            // g.shader().uniform("eye_sep", 0.02 * eye_dir);
            // g.shader().uniform("foc_len", 6);
            Color c = (eye_dir > 0)? Color{1.0f, 0.5f, 0.0f, 1.0f} : Color{0.0f, 0.5f, 1.0f, 1.0f};
            if (flags & is_omni) {
                c.g = 1.0f;
            }
            g.shader().uniform("color", c);
            g.shader().uniform("is_omni", (flags & is_omni)? 1.0f: 0.0f);

            // g.pushMatrix();
            // g.translate(0, 3, -20);
            // g.draw(m);
            // g.popMatrix();

            for (int i = -10; i < 11; i += 2) {
                for (int j = -10; j < 11; j += 2) {
                    for (int k = -10; k < 11; k += 2) {
                        g.pushMatrix();
                        g.translate(i, j, k);
                        // if (flags & coloring) {
                        //  g.color((10 - std::abs(i)) / 10.f,
                        //          (10 - std::abs(j)) / 10.f,
                        //          (10 - std::abs(k)) / 10.f);
                        // }
                        g.draw(m);
                        g.popMatrix();
                    }
                }
            }

            return;
        }


        if (flags & flicker) {
            if (frame_num % 2) {
                auto m = Matrix4f::perspectiveRight(y, ar, n, f, e, l);
                g.projMatrix(m);
                g.color(1, 0, 0);
            }
            else {
                auto m = Matrix4f::perspectiveLeft(y, ar, n, f, e, l);
                g.projMatrix(m);
                g.color(0, 0, 1);
            }
        }
        else if (flags & is_stereo) {
            if (flags & is_right_eye) {
                auto m = Matrix4f::perspectiveRight(y, ar, n, f, e, l);
                g.projMatrix(m);
                g.color(1, 0, 0);
            }
            else {
                auto m = Matrix4f::perspectiveLeft(y, ar, n, f, e, l);
                g.projMatrix(m);
                g.color(0, 0, 1);
            }
        }

        

        // if (flags & lighting) {
        //  g.lighting(true);
        // }
        // else {
        //  g.lighting(false);
        // }

        // if (!(flags & coloring)) {
        //  g.color(1);
        // }

        for (int i = -10; i < 11; i += 2) {
            for (int j = -10; j < 11; j += 2) {
                for (int k = -10; k < 11; k += 2) {
                    g.pushMatrix();
                    g.translate(i, j, k);
                    // if (flags & coloring) {
                    //  g.color((10 - std::abs(i)) / 10.f,
                    //          (10 - std::abs(j)) / 10.f,
                    //          (10 - std::abs(k)) / 10.f);
                    // }
                    g.draw(m);
                    g.popMatrix();
                }
            }
        }

        // g.color(1);
        // g.pushMatrix();
        // g.translate(0, 3, -20);
        // g.draw(m);
        // g.popMatrix();

    }

    void onKeyDown(Keyboard const& k) override
    {
        // ^= toggles bit
        if (k.key() == '1') flags ^= use_shader_displacement;
        if (k.key() == '2') flags ^= is_right_eye;
        if (k.key() == '3') flags ^= is_stereo;
        if (k.key() == '4') flags ^= flicker;
        if (k.key() == '5') flags ^= lighting;
        if (k.key() == '6') flags ^= coloring;
        if (k.key() == '7') flags ^= is_omni;
        if (k.key() == 'p') {
            cout << "________________________________________" << endl;
            cout << "use_shader: " << ((flags & use_shader_displacement)? "true" : "false") << endl;
            cout << "is_right_eye: " << ((flags & is_right_eye)? "true" : "false") << endl;
            cout << "is_stereo: " << ((flags & is_stereo)? "true" : "false") << endl;
            cout << "flicker: " << ((flags & flicker)? "true" : "false") << endl;
            cout << "lighting: " << ((flags & lighting)? "true" : "false") << endl;
            cout << "coloring: " << ((flags & coloring)? "true" : "false") << endl;
            cout << "is_omni: " << ((flags & is_omni)? "true" : "false") << endl;
        }
        if (k.key() == 'o') {
            cout << view().pose().pos() << endl;
        }
    }
};

int main()
{
    pm app;
    app.start();
}