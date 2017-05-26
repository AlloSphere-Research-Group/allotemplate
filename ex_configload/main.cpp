#include "config.hpp"
#include "al/core.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace al;
using namespace std;

class MyApp : public App {
public:
  Graphics g {*this};
  om::Config config;
  void onCreate() {
    config.load("data/projectorConfigurationTemplate.txt");
    config.print();
  }
  void onDraw() {
    g.clearColor(0, 0, 0);
  }
};

int main() {
  MyApp app;
  app.start(); // blocks
  return 0;
}