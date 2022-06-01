
// Include the header file for this source file.
#include <library_name/singing_polyhedron.hpp>

namespace library_name
{

  void SingingPolyhedron::init()
  {
    // Add a tetrahedron to our graphics mesh.
    al::addTetrahedron(mesh);
  }

  void SingingPolyhedron::onProcess(al::AudioIOData &io)
  {
    // Set our oscillator's frequency using the output from `whichNote()`.
    sineOsc.freq(SCALE[whichNote()]);
    while (io())
    {
      /// Generate the sample and reduce its amplitude by half.
      const float sample = sineOsc() / 2;

      // Feed the sample into the output channels.
      io.out(0) += sample;
      io.out(1) += sample;
    }
  }

  void SingingPolyhedron::onProcess(al::Graphics &g)
  {
    // Clear the screen.
    g.clear(0, 0, 0);
    // Draw the outline of the tetrahedron.
    g.polygonLine();

    g.pushMatrix();
    // Rotate the tetrahedron using the current value of `phase`.
    g.rotate(phase, 0, 1, 0);
    // Set the color of the tetrahedron's outline using `whichNote()` to control
    // the hue.
    g.color(
        al::Color(al::HSV((double)whichNote() / (double)SCALE_LEN, 0.4, 1), 1));
    // Draw the mesh.
    g.draw(mesh);
    g.popMatrix();
  }

  void SingingPolyhedron::update(double dt)
  {
    // Update the phase of the polyhedron.
    phase += dt / PERIOD * 360;
    if (phase > 360)
    {
      phase = 0;
    }
  }

  unsigned int SingingPolyhedron::whichNote() const
  {
    const int which = (int)phase / (360 / (SCALE_LEN * 2 - 2));
    // Convert `which` to an integer walking from 0 to SCALE_LEN - 1 and back.
    return which - (which / (SCALE_LEN - 1)) * (which % (SCALE_LEN - 1)) * 2;
  }

}; // namespace library_name
