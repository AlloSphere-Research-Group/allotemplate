#include "al/util/al_Image.hpp"
#include <iostream>

using namespace al;
using namespace std;

int main()
{
    int const w = 64;
    int const h = 64;

    Image img0;
    img0.resize(w, h, Image::LUMINANCE);
    for (int y = 0; y < h; y += 1) {
        for (int x = 0; x < w; x += 1) {
            auto& pix = img0.at(x, y);
            pix.r = x * y * 255 / w / h;
        }
    }
    img0.save("data/lum.png");
    img0.save("data/lum.bmp");
    img0.save("data/lum.jpg");
    img0.save("data/lum.tiff");

    Image img1;
    img1.resize(w, h, Image::RGB);
    for (int y = 0; y < h; y += 1) {
        for (int x = 0; x < w; x += 1) {
            auto& pix = img1.at(x, y);
            pix.r = x * 255 / w;
            pix.g = y * 255 / h;
            pix.b = 255;
        }
    }
    img1.save("data/rgb.png");
    img1.save("data/rgb.bmp");
    img1.save("data/rgb.tiff");

    Image img2;
    img2.resize(w, h, Image::RGBA);
    for (int y = 0; y < h; y += 1) {
        for (int x = 0; x < w; x += 1) {
            auto& pix = img2.at(x, y);
            pix.r = x * 255 / w;
            pix.g = y * 255 / h;
            pix.b = 255;
            pix.a = x * y * 255 / w / h;
        }
    }
    img2.save("data/rgba.png");
    img2.save("data/rgba.tiff");

    auto testLoad = [](const string& filename, const string& extension) {
        Image img;
        img.load("data/" + filename + "." + extension);
        cout << '[' << filename << ']' << '\n';
        cout << "width: " << img.width() << ", "
             << "height" << img.height() << ", "
             << "channels: " << img.channels() << '\n';
        img.save("data/output/" + filename + "_" + extension + ".png");
        img.save("data/output/" + filename + "_" + extension + ".tiff");
        if (extension != "png" && extension != "tiff") {
            img.save("data/output/" + filename + "_" + extension + "." + extension);
        }
    };

    testLoad("lum", "png");
    testLoad("lum", "bmp");
    testLoad("lum", "jpg");
    testLoad("lum", "tiff");

    testLoad("rgb", "png");
    testLoad("rgb", "bmp");
    testLoad("rgb", "tiff");

    testLoad("rgba", "png");
    testLoad("rgba", "tiff");

    return 0;
}