#include "sphere_utils.hpp"
#include "general_utils.hpp"
#include "cubemap.hpp"
#include "config.hpp"

#include "al/core.hpp"

#include <cmath>
#include <vector>

using namespace al;
using namespace std;

template<typename T0, typename T1, typename T2, typename T3>
void println(T0 t0, T1 t1, T2 t2, T3 t3) {
    std::cout << t0 << ' ' << t1 << ' ' << t2 << ' ' << t3 << std::endl;
}

class MyApp : public App {
public:
    int cuberes = 512;

    Graphics g {this};
    Viewpoint view;
    NavInputControl nav;
    VAOMesh mesh;

    CubeRender cube_render;
    vector<unique_ptr<Texture>> cubesampletexs;
    vector<CubeSampler> cube_samplers;

    om::Config om_config;

    void onInit()
    {
        if (sphere::is_renderer()) {
            int width, height;
            sphere::get_fullscreen_dimension(&width, &height);
            if (width != 0 && height != 0) {
                // cout << "width: " << width << " height: " << height << endl;
                dimensions(0, 0, width, height);
                decorated(false);
            }
            else {
                cout << "[!] calculated width and/or height are/is zero!" << endl;
            }
        }
    }

    void onCreate()
    {
        append(nav);
        nav.target(view);

        auto config_file_path = sphere::config_file_path("data/config.txt");
        om_config.load(config_file_path);
        om_config.print();
        om_config.loadData();

        if (sphere::is_renderer()) {
            cuberes = 2048;
        }
        cube_render.init(cuberes);

        // int sampletex_width = 4 * cuberes;
        // int sampletex_height = 2 * cuberes;
        // vector<float> arr = sphere::generate_equirect_sampletex(sampletex_width, sampletex_height);
        // cubesampletex.create2D(sampletex_width, sampletex_height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        // cubesampletex.submit(arr.data());

        const int nProj = om_config.numProjectors();
        cubesampletexs.resize(nProj);
        cube_samplers.resize(nProj);

        for (int i = 0; i < nProj; i += 1) {
            cubesampletexs[i] = make_unique<Texture>();
            cubesampletexs[i]->create2D(om_config.width(i), om_config.height(i), GL_RGBA32F, GL_RGBA, GL_FLOAT);
            cubesampletexs[i]->submit(om_config.data(i));

            cube_samplers[i].init();
            cube_samplers[i].sampleTexture(*cubesampletexs[i]);
            cube_samplers[i].cubemap(cube_render.cubemap);
        }

        addIcosahedron(mesh);
        auto num_verts = mesh.vertices().size();
        for (int i = 0; i < num_verts; i++) {
          mesh.color(i / float(num_verts), (num_verts - i) / float(num_verts), 0.0);
        }
        mesh.update();
    }

    void onAnimate(double dt)
    {
        nav.step();
        cube_render.pose(view);
    }

    void onDraw()
    {
        g.polygonMode(Graphics::FILL);
        g.depthTesting(true);
        g.cullFace(true);

        // bind cubemap fbo and capture 6 faces
        cube_render.begin(g);
        for (int eye = 0; eye < 1; eye += 1) {
            cube_render.set_eye(eye);
            for (int i = 0; i < 6; i += 1) {
                cube_render.set_face(i);

                g.clear(i / 5.0f, (5 - i) / 5.0f, 1.0f);
                g.pushMatrix();
                g.translate(sinf(sec()), 0, -10);
                g.rotate(sinf(2 * sec()), 0, 0, 1);
                g.rotate(sinf(3 * sec()), 0, 1, 0);
                g.scale(3, 2, 1);
                g.draw(mesh);
                g.popMatrix();
            }
        }
        cube_render.end();

        // now sample cubemap and draw result to quad
        g.clear(0);

        float w = fbWidth();
        float h = fbHeight();
        g.scissorTest(true);
        for (int i = 0 ; i < om_config.numProjectors(); i += 1) {
            auto p = om_config.projector(i);
            g.viewport(w * p.l, h * p.b, w * p.w, h * p.h);
            g.scissor(w * p.l, h * p.b, w * p.w, h * p.h);
            cube_samplers[i].draw(g);
        }
        g.viewport(0, 0, w, h);
        g.scissorTest(false);
    }

};

int main() {
    MyApp app;
    app.start();
    return 0;
}