
// Header guards prevent code from being loaded twice.
#ifndef LIBRARY_NAME_SINGING_POLYHEDRON_HPP
#define LIBRARY_NAME_SINGING_POLYHEDRON_HPP

// Include the libraries needed for this header.
#include <Gamma/Oscillator.h>
#include <al/graphics/al_Shapes.hpp>
#include <al/scene/al_PolySynth.hpp>

/// Namespace for our reusable library code.
namespace library_name {

/// A simple demo showing a spinning polyhedron with an accompanying sine wave.
class SingingPolyhedron : public al::SynthVoice {
public:
  void init() override;
  void onProcess(al::AudioIOData &io) override;
  void onProcess(al::Graphics &g) override;
  void update(double dt) override;

private:
  /// Sine wave oscillator.
  gam::Sine<> sineOsc;
  /// How many notes are in the scale.
  static const std::size_t SCALE_LEN = 5;
  /// Frequencies of notes that the oscillator will play.
  static constexpr double SCALE[SCALE_LEN] = {440., 495., 556.875, 660, 742.5};

  /// The period of the polyhedron's rotation.
  static constexpr double PERIOD = 6;

  /// Graphics mesh.
  al::Mesh mesh;
  /// The current phase of the polyhedron.
  double phase = 0;

  /// Get an index in the array `SCALE` for the singing polyhedron to play.
  unsigned int whichNote() const;
};

}; // namespace library_name

#endif
