#include <math.h>

int REDPIN, GREENPIN, BLUEPIN;

// maximum brightness for each color, use to modify final colours depending on LED choice
float r_brightness = 1;
float g_brightness = 0.7;
float b_brightness = 0.7;

int current_r, current_g, current_b = 0;

float total_brightness = 1; //the overall brightness target 0(off) --> 1(max brightness)

int current_kelvin = 0;
int iteration_delay = 30;

void setup_rgb (int r_pin, int g_pin, int b_pin, float overall_brightness) {

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

void fade_rgb(int r, int g, int b, int timespan)
{


  if(timespan < 30){
    timespan=30;
  }

  //if r,g,b currently = 0 i.e. off do a brightness fade_rgb

  double iterations = timespan/iteration_delay; //number of iterations between moves (delay between each step)
  Serial.printf("RGB Fading from (%d,%d,%d) to (%d,%d,%d) in %d ms with %f iterations\n",current_r, current_g,current_b,r,g,b,timespan,iterations);

  double rfadeAmount =  (current_r - r) / iterations;
  double gfadeAmount = (current_g - g) / iterations;
  double bfadeAmount =  (current_b - b) / iterations;

  Serial.printf("Fade amounts(%f,%f,%f)\n", rfadeAmount,gfadeAmount,bfadeAmount);
  double new_r, new_g, new_b;

  new_r = current_r;
  new_g = current_g;
  new_b = current_b;
  double start, elapsed;
  start = millis();
  for(double i = 0; i<iterations; i++)
  {
     new_r = new_r - rfadeAmount;
     new_g = new_g - gfadeAmount;
     new_b = new_b - bfadeAmount;
     set_rgb(new_r, new_g, new_b);
     //Serial.printf("(%f,%f,%f)\n", new_r,new_g,new_b);

     // wait for 30 milliseconds to see the dimming effect
     delay(30);
  }

  set_rgb(r, g, b); //make sure were bang on at the end
  elapsed = millis() - start; //crashing somewhere here !!!!
  Serial.printf("Fade took %f ms\n", elapsed);
  Serial.println("Fading Complete");

}

void fade_kelvin(int to_kelvin, int timespan)
{

  Serial.println("Kelvin Fading");
  if(timespan < 30){timespan=30;}
  int iterations = timespan/iteration_delay; //number of iterations between moves (30ms delay
  float fadeAmount =  (current_kelvin - to_kelvin) / iterations;
  for(int i=0; i<iterations;i++)
    {
      set_colour_temperature(current_kelvin-fadeAmount);
      // wait for 30 milliseconds to see the dimming effect
      delay(iteration_delay);
    }

    Serial.println("Fading Complete");
}

void rgb_apply(double r, double g, double b)
{
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);

    current_r = r; //store latest values for fading purposes
    current_g = g;
    current_b = b;

    //Serial.println(r);
    //Serial.println(g);
    //Serial.println(b);

    analogWrite(REDPIN, current_r * total_brightness);
    analogWrite(GREENPIN, current_g * total_brightness);
    analogWrite(BLUEPIN, current_b * total_brightness);
}
void kelvin_apply(int r, int g, int b)
{
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);

    current_r = r * r_brightness * total_brightness; //store latest values for fading purposes
    current_g = g * g_brightness * total_brightness;
    current_b = b * b_brightness * total_brightness;

    analogWrite(REDPIN, current_r);
    analogWrite(GREENPIN, current_g);
    analogWrite(BLUEPIN, current_b);
}

void set_rgb(int r, int g, int b)
{
  //will reset kelvin value as well, so that proper fading can occur
   rgb_apply(r,g,b);
   current_kelvin = 0;
}

void set_brightness(float brightness)
{
  //change the overall brightness of the lights
  total_brightness = brightness;
  if(current_kelvin == 0)
  {
    set_rgb(current_r,current_g,current_b);
  }
  else
  {
    set_colour_temperature(current_kelvin);
  }

}

void rgb_off()
{
  set_rgb(0,0,0);
}

void set_colour_temperature(int kelvin)
{
    int rgb_data[3];
    kelvin_to_rgb(kelvin, rgb_data);
    kelvin_apply(rgb_data[0],rgb_data[1],rgb_data[2]);
}

void kelvin_to_rgb(int tmpKelvin,int rgb[])
{
  float tmpCalc = 0;
  byte r, g, b = 0;
  current_kelvin = tmpKelvin;
    //Temperature must fall between 1000 and 40000 degrees
    if(tmpKelvin < 1000){tmpKelvin = 1000;}
    if(tmpKelvin > 40000){tmpKelvin = 40000;}

    //All calculations require tmpKelvin \ 100, so only do the conversion once
    tmpKelvin = tmpKelvin / 100;

    //RED Calc
    if(tmpKelvin <= 66) r = 255;
    else
    {
        //Note: the R-squared value for this approximation is .988
        tmpCalc = tmpKelvin - 60;
        tmpCalc = 329.698727446 * pow(tmpCalc,-0.1332047592);
        r = tmpCalc;

    }

    //GREEN Calc
    if(tmpKelvin <= 66)
    {
        //Note: the R-squared value for this approximation is .996
        tmpCalc = tmpKelvin;
        tmpCalc = (99.4708025861 * log(tmpCalc)) - 161.1195681661;
        g = tmpCalc;
    }
    else
    {
        //Note: the R-squared value for this approximation is .987
        tmpCalc = tmpKelvin - 60;
        tmpCalc = 288.1221695283 * pow(tmpCalc,-0.0755148492);
        g = tmpCalc;
    }

    //BLUE Calc
    if(tmpKelvin >= 66) b = 255;
    else if(tmpKelvin <= 19)  b = 0;
    else
    {
        //Note: the R-squared value for this approximation is .998
        tmpCalc = tmpKelvin - 10;
        tmpCalc = (138.5177312231 * log(tmpCalc)) - 305.0447927307;
        b = tmpCalc;
    }
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
  }
