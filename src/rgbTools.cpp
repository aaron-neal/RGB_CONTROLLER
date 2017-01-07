#include "rgbTools.h"

int redPin, greenPin, bluePin;

/* maximum brightness for the LEDs
    0(off) --> 1(max brightness)  */
double _brightness = 1; //consider a non-linear version for better brightness control

//globals
rgb _currentRGB;
int _lastUpdate = 0;
int _iterationDelay = 10; //ms between step colour changes
int _fadeSteps = 0;

/* RGB Fading variables*/
bool _fadingRGB = false;
rgb _initialColour;
double _rgbFadeAmounts[3];

/* Kelvin Fading variables*/
bool _fadingKelvin = false;
int _currentKelvin = 0;
double _kelvinFadeAmount;

void setupRGB (int rPin, int gPin, int bPin) {
  redPin = rPin;
  greenPin = gPin;
  bluePin = bPin;

  analogWriteRange(255); //use 0 --> 255 as pwm range
  analogWriteFreq(1000); //set PWM freq.
  delay(50);
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void fadeRGB(rgb fadeRGB, int timespan) {
  timespan = max(timespan,_iterationDelay);
  _fadingRGB = true;
  _fadingKelvin = false;
  _fadeSteps = timespan/_iterationDelay;
  _rgbFadeAmounts[0] =  (_currentRGB.r - fadeRGB.r) / _fadeSteps;
  _rgbFadeAmounts[1] = (_currentRGB.g - fadeRGB.g) / _fadeSteps;
  _rgbFadeAmounts[2] =  (_currentRGB.b - fadeRGB.b) / _fadeSteps;
}

void rgbLoop()
{
  unsigned long current_millis = millis();
  if (_fadingRGB || _fadingKelvin) {
    // Enough time since the last step ?
    if (current_millis - _lastUpdate >= _iterationDelay) {
      if (_fadeSteps > 0) {
        //still fading steps left
        if(_fadingRGB){
          _currentRGB.r = _currentRGB.r - _rgbFadeAmounts[0];
          _currentRGB.g = _currentRGB.g - _rgbFadeAmounts[1];
          _currentRGB.b = _currentRGB.b - _rgbFadeAmounts[2];
        }
        if(_fadingKelvin){
          _currentKelvin = _currentKelvin-_kelvinFadeAmount;
          _currentRGB = (kelvinToRGB(_currentKelvin));
        }
        _fadeSteps--;
      } else {
        _fadingRGB = false;
        _fadingKelvin = false;
      }
      _lastUpdate = current_millis;
    }
  }
  //always set colours
  analogWrite(redPin, _currentRGB.r * _brightness);
  analogWrite(greenPin, _currentRGB.g * _brightness);
  analogWrite(bluePin, _currentRGB.b * _brightness);
}

void fadeKelvin(int toKelvin, int timespan)
{
  timespan = max(timespan,_iterationDelay); //limit smallest timespan to be iteration delay
  if(_currentKelvin != 0)
  {
    //fade kelvin
    _fadingKelvin = true;
    _fadingRGB = false;
    _fadeSteps = timespan/_iterationDelay; //number of iterations between moves (30ms delay
    _kelvinFadeAmount =  (_currentKelvin - toKelvin) / _fadeSteps;
  } else {
    //fade RGB
    fadeRGB(kelvinToRGB(toKelvin),timespan);
  }
}

void fadeRGBBlocking(rgb fadeRGB, int timespan){
 timespan = max(timespan,_iterationDelay); //limit smallest timespan to be iteration delay
 double iterations = timespan/_iterationDelay; //number of iterations between moves (delay between each step)

 double rfadeAmount =  (_currentRGB.r - fadeRGB.r) / iterations;
 double gfadeAmount = (_currentRGB.g - fadeRGB.g) / iterations;
 double bfadeAmount =  (_currentRGB.b - fadeRGB.b) / iterations;

 for(double i = 0; i<iterations; i++)
 {
    _currentRGB.r = _currentRGB.r - rfadeAmount;
    _currentRGB.g = _currentRGB.g - gfadeAmount;
    _currentRGB.b = _currentRGB.b - bfadeAmount;

    analogWrite(redPin, _currentRGB.r * _brightness);
    analogWrite(greenPin, _currentRGB.g * _brightness);
    analogWrite(bluePin, _currentRGB.b * _brightness);

    delay(_iterationDelay);
 }
 _currentRGB = fadeRGB;
}

void setRGB(rgb setRGB)
{
   //will reset kelvin value as well, so that proper fading can occur
   _fadingRGB = false;
   _fadingKelvin = false;
   _currentKelvin = 0;

   setRGB.r = constrain(setRGB.r, 0, 255);
   setRGB.g = constrain(setRGB.g, 0, 255);
   setRGB.b = constrain(setRGB.b, 0, 255);
   _currentRGB = setRGB; //store latest values for fading purposes

}

void setBrightness(double brightness)
{
  //change the overall brightness of the lights
  _brightness = brightness;
}

void off()
{
  setRGB({0,0,0});
}

void setKelvin(int kelvin)
{
    _currentKelvin = kelvin;
    setRGB(kelvinToRGB(kelvin));
}
