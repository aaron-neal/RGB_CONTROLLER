#include "rgbTools.h"



int REDPIN, GREENPIN, BLUEPIN;

/* maximum brightness for each color, use to
modify final colours depending on LED choice*/
double r_brightness = 1;
double g_brightness = 1;
double b_brightness = 1;
double total_brightness = 1; //the overall brightness target 0(off) --> 1(max brightness)

//globals
rgb currentRGB;
int current_kelvin = 0;


int iteration_delay = 10; //ms between step colour changes

void setupRGB (int r_pin, int g_pin, int b_pin, double overall_brightness) {

  REDPIN = r_pin;
  GREENPIN = g_pin;
  BLUEPIN = b_pin;
  total_brightness = overall_brightness;

  analogWriteRange(255); //use 0 --> 255 as pwm range
  analogWriteFreq(100); //set PWM freq. to 100hz
  delay(50);
  rgb_off(); //turn lights off
  delay(50);

}


void rgbLoop()
{

}

void fade_rgb(rgb fadeRGB, int timespan)
{
  timespan = max(timespan,iteration_delay); //limit smallest timespan to be iteration delay
  //if r,g,b currently = 0 i.e. off do a brightness fade_rgb
  double iterations = timespan/iteration_delay; //number of iterations between moves (delay between each step)
  //Serial.printf("RGB Fading from (%d,%d,%d) to (%d,%d,%d) in %d ms with %f iterations\n",current_r, current_g,current_b,r,g,b,timespan,iterations);

  double rfadeAmount =  (currentRGB.r - fadeRGB.r) / iterations;
  double gfadeAmount = (currentRGB.g - fadeRGB.g) / iterations;
  double bfadeAmount =  (currentRGB.b - fadeRGB.b) / iterations;

  //Serial.printf("Fade amounts(%f,%f,%f)\n", rfadeAmount,gfadeAmount,bfadeAmount);
  rgb newRGB;

  newRGB.r = currentRGB.r;
  newRGB.g = currentRGB.g;
  newRGB.b = currentRGB.b;

  double start, elapsed;
  start = millis();
  for(double i = 0; i<iterations; i++)
  {
     newRGB.r = newRGB.r - rfadeAmount;
     newRGB.g = newRGB.g - gfadeAmount;
     newRGB.b = newRGB.b - bfadeAmount;
     set_rgb(newRGB);
     //Serial.printf("(%f,%f,%f)\n", new_r,new_g,new_b);

     delay(iteration_delay);
  }

  set_rgb(fadeRGB); //make sure were bang on at the end
  elapsed = millis() - start; //crashing somewhere here !!!!
  //Serial.printf("Fade took %f ms\n", elapsed);
  //Serial.println("Fading Complete");
}

void fade_kelvin(int to_kelvin, int timespan)
{
  //Serial.println("Kelvin Fading");
  timespan = max(timespan,iteration_delay); //limit smallest timespan to be iteration delay
  int iterations = timespan/iteration_delay; //number of iterations between moves (30ms delay
  float fadeAmount =  (current_kelvin - to_kelvin) / iterations;
  for(int i=0; i<iterations;i++)
    {
      set_colour_temperature(current_kelvin-fadeAmount);
      delay(iteration_delay);
      yield();
    }
    //Serial.println("Fading Complete");
}

void rgb_apply(rgb applyRGB, bool colourAdjust)
{
  applyRGB.r = constrain(applyRGB.r, 0, 255);
  applyRGB.g = constrain(applyRGB.g, 0, 255);
  applyRGB.b = constrain(applyRGB.b, 0, 255);

  currentRGB = applyRGB; //store latest values for fading purposes

  if(colourAdjust){
    currentRGB.r = applyRGB.r * r_brightness;
    currentRGB.g = applyRGB.g * g_brightness;
    currentRGB.b = applyRGB.b * b_brightness;
  }

  analogWrite(REDPIN, currentRGB.r * total_brightness);
  analogWrite(GREENPIN, currentRGB.g * total_brightness);
  analogWrite(BLUEPIN, currentRGB.b * total_brightness);

}

void set_rgb(rgb setRGB)
{
   //will reset kelvin value as well, so that proper fading can occur
   rgb_apply(setRGB,false);
   current_kelvin = 0;
}

void set_brightness(double brightness)
{
  //change the overall brightness of the lights
  total_brightness = brightness;
  if(current_kelvin == 0)
  {
    set_rgb(currentRGB);
  }
  else
  {
    set_colour_temperature(current_kelvin);
  }

}

void rgb_off()
{
  analogWrite(REDPIN, 0);
  analogWrite(GREENPIN, 0);
  analogWrite(BLUEPIN, 0);
}

void set_colour_temperature(int kelvin)
{
    current_kelvin = kelvin;
    rgb rgbData;
    rgbData = kelvinToRGB(kelvin);
    rgb_apply(rgbData,true);
}
