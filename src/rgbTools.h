#ifndef RGB_TOOLS
  #define RGB_TOOLS
  #include <Arduino.h>
  #include "colours.h"

  void setupRGB (int rPin, int gPin, int bPin, int wPin);
  void fadeKelvin(int toKelvin, int timespan);
  void setRGB(rgb setRGB);
  void setBrightness(double brightness);
  void setKelvin(int kelvin);
  void rgbLoop ();
  void fadeRGB(rgb fadeRGB, int timespan);
  void fadeRGBBlocking(rgb fadeRGB, int timespan);
  void startRandom(int timespan);
  void stopRandom();
  rgb randomColour();
  void off();
#endif
