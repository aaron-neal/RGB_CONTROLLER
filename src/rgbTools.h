#ifndef RGB_TOOLS
  #define RGB_TOOLS // To make sure you don't declare the function more than once by including the header multiple times.
  #include "Arduino.h"
  #include "colours.h"
  #include <math.h>
  #include "colours.h"

  void setupRGB (int r_pin, int g_pin, int b_pin, double initialBrightness);
  void fadeKelvin(int to_kelvin, int timespan);
  void setRGB(rgb setRGB);
  void setBrightness(double brightness);
  void setKelvin(int kelvin);
  void rgbLoop ();
  void fadeRGB(rgb fadeRGB, int timespan);
  void fadeRGBBlocking(rgb fadeRGB, int timespan);
  void off();
#endif
