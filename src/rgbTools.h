#ifndef RGB_TOOLS
  #define RGB_TOOLS // To make sure you don't declare the function more than once by including the header multiple times.
  #include "Arduino.h"
  #include "colours.h"
  #include <math.h>
  #include "colours.h"

  void setupRGB (int r_pin, int g_pin, int b_pin, double overall_brightness);
  void fade_rgb(rgb fadeRGB, int timespan);
  void fade_kelvin(int to_kelvin, int timespan);
  void rgb_apply(rgb applyRGB, bool colourAdjust);
  void set_rgb(rgb setRGB);
  void set_brightness(double brightness);
  void rgb_off();
  void set_colour_temperature(int kelvin);
  void rgbLoop ();

#endif
