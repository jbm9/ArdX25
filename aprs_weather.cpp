#include <stdio.h>
#include "aprs_weather.h"

// Define our APRS Symbol, APRS101.pdf Appendix 2, p105 (115)
#define SYMBOL_PAGE '/'  // main page
#define SYMBOL_SYMBOL 'O' // Balloon!  

void APRSWeather::set_gps_handler(GPSHandler *g) {
  gpsh = g;
}

void APRSWeather::note_temp_internal(uint8_t t) {
  temp_internal = t;
}

void APRSWeather::note_temp_external(uint8_t t) {
  temp_external = t;
}

void APRSWeather::note_pressure(uint8_t t) {
  pressure = t;
}

void APRSWeather::note_acc(uint8_t x, uint8_t y, uint8_t z) {
  acc_x = x;
  acc_y = y;
  acc_z = z;
}

// Using an AREF=3.3V, with 8b outputs, convert the reading into
// degrees Fahrenheit, which APRS insists on using.  This formula uses
// integer math only, and yields an accuracy of -1~+3 degF over the
// entire range, with a mean error of -0.018, sd=0.1898.
int8_t APRSWeather::temp_to_fahrenheit(uint8_t t) {
 
  int16_t retval = 0;

#if USE_RAW_3V3_AREF
  int16_t tp = t - 199;
  if (t > 252) tp = 252-199; // this overflows a signed int
  retval = (2*tp + tp/3 + 2);
#else
  // mean error 0.002, sd(epsilon)=0.037, max err is -1/+2degF
  // 2*(a-188)+floor((a-188)/2)-floor((a-188)/20) - 3 

  int16_t tp = t - 207;

  retval = 2*tp + tp/5 - 1;
#endif

  return (int8_t)retval;
}

int8_t APRSWeather::temp_internal_fahrenheit() {
  return temp_to_fahrenheit(temp_internal);
}

int8_t APRSWeather::temp_external_fahrenheit() {
  return temp_to_fahrenheit(temp_external);
}


// With AREF=3.3V and V_S=5.0V (weird, but how the board has to be),
// this formula yields a mean error of -0.017, sd=0.1238.
//
// Note that the maximum pressure is 82kPa, which is ~0.8atm
//
// returns millibars, need to tack on a zero for APRS
uint16_t APRSWeather::pressure_millibars() {
#if USE_RAW_3V3_AREF
  uint16_t retval = 105 + 2*pressure + 3*pressure/5;
  return retval;
#else
  uint16_t q = pressure +25 ;
  uint16_t retval = (q/2 - q/13)*10;
  return retval;
#endif
}

// APRS101.pdf, Chapter 12 (p65 / PDF page 75)
uint16_t APRSWeather::format_string(char *buf, uint16_t maxbuf) {
  uint16_t pos = 0;

  *(buf+pos) = ';';
  pos++;

  char OBJECT_NAME[9] = { 'A', 'J', '9', 'B', 'M', '-','1', '1', ' ' };

  memcpy((void*)(buf+pos), OBJECT_NAME, 9);
  pos += 9;

  *(buf+pos) = '*';
  pos++;
      

  snprintf(buf+pos, 7+1, "%04i00h", gpsh->hhmm);
  pos += 7;

  snprintf(buf+pos, 5+1, "%04i.", abs(gpsh->lat_whole));
  pos += 5;
  snprintf(buf+pos, 2+1, "%02i", gpsh->lat_frac);
  pos += 2;
  *(buf+pos) = (gpsh->lat_whole > 0 ? 'N' : 'S');
  pos++;

  *(buf+pos) = '/';
  pos++;

  snprintf(buf+pos, 6+1, "%05i.", abs(gpsh->lon_whole));
  pos += 6;
  snprintf(buf+pos, 2+1, "%02d", gpsh->lon_frac);
  pos += 2;
  *(buf+pos) = (gpsh->lon_whole > 0 ? 'E' : 'W');
  pos++;

  *(buf+pos) = '_';
  pos++;

  // Wind speed, which we don't have.
  for (uint8_t i = 0; i < 3; i++) {
    *(buf+pos) = '.';
    pos++;
  }
  *(buf+pos) = '/';
  pos++;

  // Wind heading, which we don't have.
  for (uint8_t i = 0; i < 3; i++) {
    *(buf+pos) = '.';
    pos++;
  }    



  // Pack up Weather data

  *(buf+pos) = 't';
  pos++;
  // External temperature
  snprintf(buf+pos, 3+1, "%03d", temp_external_fahrenheit());
  pos += 3;

  *(buf+pos) = 'b';
  pos ++;

  snprintf(buf+pos, 5+1, "%04d0", pressure_millibars());
  pos += 5;

  return pos;    
}


