/*
 *
 * aprs_weather.cpp: APRS Weather packet formatter
 *
 * Copyright (c) 2012 Applied Platonics, LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 only.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include "aprs_weather.h"

// Define our APRS Symbol, APRS101.pdf Appendix 2, p105 (115)
#define SYMBOL_PAGE '/'  // main page
#define SYMBOL_SYMBOL 'O' // Balloon!  

/**
 * Silly accessor method for GPS Handler kludge.
 * This probably ought to be replaced by a better pattern.
 *
 * @param[in] g Pointer to a GPS Handler you'll keep up-to-date yourself
 *
 * @return Void.
 */
void APRSWeather::set_gps_handler(GPSHandler *g) {
  gpsh = g;
}


/**
 * Accessor method to set the given reading of the internal
 * temperature sensor.  The value is mapped to an actual temperature
 * in temp_to_fahrenheit()
 *
 * @param[in] t The upper 8b of the ADC reading.
 *
 * @return Void.
 */
void APRSWeather::note_temp_internal(uint8_t t) {
  temp_internal = t;
}


/**
 * Accessor method to set the given reading of the external
 * temperature sensor.  The value is mapped to an actual temperature
 * in temp_to_fahrenheit()
 *
 * @param[in] t The upper 8b of the ADC reading.
 */
void APRSWeather::note_temp_external(uint8_t t) {
  temp_external = t;
}


/**
 * Accessor method to set the given reading of the pressure sensor.
 * The value is mapped to an actual temperature in
 * pressure_millibars().
 *
 * @param[in] t The upper 8b of the ADC reading.
 */
void APRSWeather::note_pressure(uint8_t t) {
  pressure = t;
}


/**
 * Accessor method to set the given reading of the accelerometer.
 *
 * @param[in] x The upper 8b of the x-axis ADC reading.
 * @param[in] y The upper 8b of the y-axis ADC reading.
 * @param[in] z The upper 8b of the z-axis ADC reading.
 *
 * @return Void.
 */
void APRSWeather::note_acc(uint8_t x, uint8_t y, uint8_t z) {
  acc_x = x;
  acc_y = y;
  acc_z = z;
}


/**
 * Converts a given temperature reading to degrees Fahrenheit.
 *
 * There are two cases here: with a direct 3.3V AREF, or with AREF run
 * through a 1.8k resistor.  The latter is done to allow the use of
 * the 5V internal AREF when reading the pressure sensor, which is a
 * 5V part.  If we don't do so, the sensor maxes out at ~80kPA/0.8atm.
 *
 * 3.3V mode: absolute error of -1~+3degF, mu(e)=-0.018, sd(e)=0.190
 *
 * 3.3V/5V mode: abs error of -1~+2degF, mu(e)=0.002, sd(e)=0.037
 *
 * @param[in]  t The raw ADC reading (upper 8b)
 *
 * @return Signed int representing the whole degrees fahrenheit.
 */
int8_t APRSWeather::temp_to_fahrenheit(uint8_t t) {
  int16_t retval = 0;

#if USE_RAW_3V3_AREF
  int16_t tp = t - 199;
  if (t > 252) tp = 252-199; // this overflows a signed int
  retval = (2*tp + tp/3 + 2);
#else
  int16_t tp = t - 207;

  retval = 2*tp + tp/5 - 1;
#endif

  return (int8_t)retval;
}

/**
 * Wrapper around temp_to_fahrenheit() for the internal sensor.
 *
 * @return Signed int representing the whole degrees fahrenheit (internal temperature).
 */
int8_t APRSWeather::temp_internal_fahrenheit() {
  return temp_to_fahrenheit(temp_internal);
}


/**
 * Wrapper around temp_to_fahrenheit() for the external sensor.
 *
 * @return Signed int representing the whole degrees fahrenheit (external temperature).
 */
int8_t APRSWeather::temp_external_fahrenheit() {
  return temp_to_fahrenheit(temp_external);
}


// With AREF=3.3V and V_S=5.0V (weird, but how the board has to be),
// this formula yields a mean error of -0.017, sd=0.1238.
//
// Note that the maximum pressure is 82kPa, which is ~0.8atm
//
// returns millibars, need to tack on a zero for APRS


/**
 * Converts the pressure reading to millibars.
 *
 * There are two cases here: with a direct 3.3V AREF, or with AREF run
 * through a 1.8k resistor.  The latter is done to allow the use of
 * the 5V internal AREF when reading the pressure sensor, which is a
 * 5V part.  If we don't do so, the sensor maxes out at ~80kPA/0.8atm.
 *
 * 3.3V mode: absolute error of [TODO], mu(e)=-0.017, sd(e)=0.124
 *
 * 3.3V/5V mode: abs err of, mu(e)= sd(e)= [TODO recompute these]
 *
 * @return An unsigned 16 value representing the pressure in millibars.
 */
uint16_t APRSWeather::pressure_millibars() {
#if USE_RAW_3V3_AREF
  return 105 + 2*pressure + 3*pressure/5;

#else
  uint16_t q = pressure + 25;
  uint16_t retval = (q/2 - q/13)*10;
  return retval;
#endif
}

/**
 * Creates an APRS-formatted Object report.

 * Specifically, it's a
 *  "Complete Weather Report Format — with Lat/Long position and Timestamp"
 * from APRS101.pdf, Chapter 12 (p66, page 76 of the PDF)
 *
 * @param[in,out] buf Pointer to a pre-allocated character string to fill in
 * @param[in]    maxbuf Shamefully unused.
 *
 * @return The length of the total message body (doesn't place final NUL)
 *
 */
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


