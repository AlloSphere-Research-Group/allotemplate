#include "al/core.hpp"

using namespace al;
using namespace std;

// glDrawBuffers(GLsizei n, GLenum const* bufs);

// GL_NONE
// GL_FRONT_LEFT
// GL_FRONT_RIGHT
// GL_BACK_LEFT
// GL_BACK_RIGHT
// GL_COLOR_ATTACHMENTn


void drawBuffer(unsigned int attachment) {
    glDrawBuffers(1, &attachment);
}



struct MyApp : App
{
    const unsigned char draw_stereo = 1 << 0;
    unsigned char flags = 0;

    Mesh m;

    bool is(unsigned int flag) {
        return (flags & flag);
    }

    void onCreate() override
    {
        addSphere(m);
        m.generateNormals();
        const int N = m.vertices().size();
        auto& colors = m.colors();
        colors.resize(N);
        for (int i = 0; i < N; i += 1) {
            colors[i].set(rnd::uniform(), rnd::uniform(), rnd::uniform());
        }
        lens().eyeSep(0.1);
        lens().focalLength(3);
        lens().fovy(60);
    }

    void onDraw(Graphics& g) override
    {
        static unsigned int frame_num = ~0;
        frame_num += 1;

        g.depthTesting(true);

        if (is(draw_stereo)) {
            if (frame_num % 2) {
                drawBuffer(GL_BACK_LEFT);
                g.clear(0);
                g.eye(Graphics::LEFT_EYE);
                g.tint(1, 0, 0);
                g.meshColor();
                g.draw(m);
            }
            else {
                drawBuffer(GL_BACK_RIGHT);
                g.clear(0);
                g.eye(Graphics::RIGHT_EYE);
                g.tint(0, 0, 1);
                g.meshColor();
                g.draw(m);
            }
        }
        else {
            drawBuffer(GL_BACK_LEFT);
            g.clear(0);
            g.eye(Graphics::MONO_EYE);
            g.tint(0, 1, 0);
            g.meshColor();
            g.draw(m);
        }
    }

    void onKeyDown(Keyboard const& k) override
    {
        // ^= toggles bit
        if (k.key() == '1') flags ^= draw_stereo;
    }
};

int main()
{
    MyApp app;
    // app.stereo(true);
    app.start();
}