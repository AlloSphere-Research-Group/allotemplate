#include "al/core.hpp"
#include <iostream>
#include <string>
#include <fstream>

std::string get_file_string(std::string path) {
  std::ifstream fstream(path); // shouldn't be rvalue
  return std::string {
    std::istreambuf_iterator<char> { fstream }, // from
    std::istreambuf_iterator<char> {} // to (0 argument returns end iterator)
  };
}

using namespace al;
using namespace std;

class MyApp : public App {
public:
  void onCreate() {
    cout << get_file_string("data/test.txt") << endl;
  }

  void onAnimate(double dt) {

  }

  void onDraw() {
    auto& g = graphics();
    g.clear(0, 1, 1);
  }

};

int main() {
  MyApp app;
  app.start(); // blocks
  return 0;
}