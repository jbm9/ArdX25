/*
 *
 * aprs_weather.h: APRS Weather formatting utility class
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

#ifndef APRS_WEATHER_H
#define APRS_WEATHER_H

#include "gps_handler.h"

/**
 * @class APRSWeather
 * @brief APRS WX object
 * 
 * This is a wrapper that contains the storage and conversion of
 * weather data.  It has a slightly messy interface, as it expects you
 * to feed it specific ADC values, then it knows how to convert those
 * to human units.
 *
 * This should probably be refactored a bit.
 */
class APRSWeather {
 private:
  // Raw ADC Readings
  uint8_t temp_internal, temp_external;
  uint8_t pressure;
  uint8_t acc_x, acc_y, acc_z;

  GPSHandler *gpsh;

 public:
  void set_gps_handler(GPSHandler *g);

  void note_temp_internal(uint8_t t);
  void note_temp_external(uint8_t t);
  void note_pressure(uint8_t t);
  void note_acc(uint8_t x, uint8_t y, uint8_t z);

  int8_t temp_to_fahrenheit(uint8_t t);
  int8_t temp_internal_fahrenheit();
  int8_t temp_external_fahrenheit();

  uint16_t pressure_millibars();

  uint16_t format_string(char *buf, uint16_t maxbuf);
};

#endif
