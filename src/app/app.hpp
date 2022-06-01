
#ifndef APPLICATION_NAME_APP_HPP
#define APPLICATION_NAME_APP_HPP

// Include headers from Allolib.
#include <al/app/al_App.hpp>
#include <al/ui/al_ControlGUI.hpp>

// Include reusable modules from the library half.
#include <library_name/singing_polyhedron.hpp>

/// Namespace for our application code.
namespace application_name {

/// Our Allolib application.
class App : public al::App {
public:
  void onCreate() override;
  void onAnimate(double dt) override;
  void onDraw(al::Graphics &g) override;
  void onSound(al::AudioIOData &io) override;

private:
  /// An example of a synth voice with graphics.
  library_name::SingingPolyhedron singingPolyhedron;
};

}; // namespace application_name

#endif
