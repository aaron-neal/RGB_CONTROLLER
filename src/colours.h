#ifndef COLOURS
  #define COLOURS
  #include <math.h>
  struct rgb {
      double r;       // percent
      double g;       // percent
      double b;       // percent
  };

  struct hsv {
      double h;       // angle in degrees
      double s;       // percent
      double v;       // percent
  };

  static hsv rgb2hsv(rgb in);
  static rgb hsv2rgb(hsv in);
  static rgb kelvinToRGB(int kelvin);

  hsv rgb2hsv(rgb in)
  {
      hsv         out;
      double      min, max, delta;

      min = in.r < in.g ? in.r : in.g;
      min = min  < in.b ? min  : in.b;

      max = in.r > in.g ? in.r : in.g;
      max = max  > in.b ? max  : in.b;

      out.v = max;                                // v
      delta = max - min;
      if( max > 0.0 ) {
          out.s = (delta / max);                  // s
      } else {
          // r = g = b = 0                        // s = 0, v is undefined
          out.s = 0.0;
          out.h = NAN;                            // its now undefined
          return out;
      }
      if( in.r >= max )                           // > is bogus, just keeps compilor happy
          out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
      else
      if( in.g >= max )
          out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
      else
          out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

      out.h *= 60.0;                              // degrees

      if( out.h < 0.0 )
          out.h += 360.0;

      return out;
  }

  rgb hsv2rgb(hsv in)
  {
      double      hh, p, q, t, ff;
      long        i;
      rgb         out;

      if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
          out.r = in.v;
          out.g = in.v;
          out.b = in.v;
          return out;
      }
      hh = in.h;
      if(hh >= 360.0) hh = 0.0;
      hh /= 60.0;
      i = (long)hh;
      ff = hh - i;
      p = in.v * (1.0 - in.s);
      q = in.v * (1.0 - (in.s * ff));
      t = in.v * (1.0 - (in.s * (1.0 - ff)));

      switch(i) {
      case 0:
          out.r = in.v;
          out.g = t;
          out.b = p;
          break;
      case 1:
          out.r = q;
          out.g = in.v;
          out.b = p;
          break;
      case 2:
          out.r = p;
          out.g = in.v;
          out.b = t;
          break;

      case 3:
          out.r = p;
          out.g = q;
          out.b = in.v;
          break;
      case 4:
          out.r = t;
          out.g = p;
          out.b = in.v;
          break;
      case 5:
      default:
          out.r = in.v;
          out.g = p;
          out.b = q;
          break;
      }
      return out;
  }

  rgb kelvinToRGB(int kelvin) {
      rgb kelvin_rgb;

      float tmpCalc = 0;

      //Temperature must fall between 1000 and 40000 degrees
      if(kelvin < 1000){kelvin = 1000;}
      if(kelvin > 40000){kelvin = 40000;}

      //All calculations require kelvin \ 100, so only do the conversion once
      kelvin = kelvin / 100;

      //RED Calc
      if(kelvin <= 66) kelvin_rgb.r = 255;
      else
      {
          //Note: the R-squared value for this approximation is .988
          tmpCalc = kelvin - 60;
          tmpCalc = 329.698727446 * pow(tmpCalc,-0.1332047592);
          kelvin_rgb.r = tmpCalc;

      }

      //GREEN Calc
      if(kelvin <= 66)
      {
          //Note: the R-squared value for this approximation is .996
          tmpCalc = kelvin;
          tmpCalc = (99.4708025861 * log(tmpCalc)) - 161.1195681661;
          kelvin_rgb.g = tmpCalc;
      }
      else
      {
          //Note: the R-squared value for this approximation is .987
          tmpCalc = kelvin - 60;
          tmpCalc = 288.1221695283 * pow(tmpCalc,-0.0755148492);
          kelvin_rgb.g = tmpCalc;
      }

      //BLUE Calc
      if(kelvin >= 66) kelvin_rgb.b = 255;
      else if(kelvin <= 19)  kelvin_rgb.b = 0;
      else
      {
          //Note: the R-squared value for this approximation is .998
          tmpCalc = kelvin - 10;
          tmpCalc = (138.5177312231 * log(tmpCalc)) - 305.0447927307;
          kelvin_rgb.b = tmpCalc;
      }
      return kelvin_rgb;
  }
#endif
