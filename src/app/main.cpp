
#include "app.hpp"

int main() {
  /// An instance of our app.
  application_name::App app;
  // Set our app's dimensions to 600 x 400.
  app.dimensions(600, 400);
  // Configure our app's audio.
  app.configureAudio(48000., 512, 2, 0);

  // Start the app!
  app.start();
}
