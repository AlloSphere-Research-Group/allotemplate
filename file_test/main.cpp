#include "al/core.hpp"
#include <iostream>
#include <string>
#include <fstream>

std::string file_to_string(std::string path) {
  std::ifstream f(path); // f couldn't be rvalue
  if (!f.is_open()) {
    std::cout << "couldn't find " << path << std::endl;
    return "";
  }
  else return std::string {
    std::istreambuf_iterator<char> {f}, // from
    std::istreambuf_iterator<char> {} // to (0 argument returns end iterator)
  };
}

using namespace al;
using namespace std;

class MyApp : public App {
public:
  void onCreate() {
    cout << file_to_string("data/test.txt") << endl;
  }

  void onAnimate(double dt) {

  }

  void onDraw() {
    auto& g = graphics();
    g.clearColor(0, 1, 1);
  }

};

int main() {
  MyApp app;
  app.start(); // blocks
  return 0;
}