/*
 *
 * gps_handler.h:GPS parser/storage utility class
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

#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define GPSHANDLER_BUFLEN 90 // Looks like 82 is the max GPGGA, but pad it out.

/**
 * @class GPSHandler
 * @brief Container/parser for GPS data
 *
 * This is a container that takes in GPS data one byte at a time, then
 * builds up and parses the sentence.  Note that it doesn't use
 * floating point, but instead stores the lat/long values as pairs of
 * 16b integers, one for the whole part, the other for the fraction.
 * The sentence's N/S and E/W data is turned into the sign of the
 * whole part.
 *
 * This is a bit ugly, but it turns 19 bytes of RAM into 8.
 *
 */
class GPSHandler {
public:
  uint8_t state;
  uint8_t expression[6];
  uint8_t bufpos;
  uint8_t buf[GPSHANDLER_BUFLEN];

  bool lock;

  int16_t lat_whole, lon_whole;
  uint8_t lat_frac, lon_frac;
  uint32_t altitude;

  uint16_t hhmm;

  GPSHandler();

  bool saw(uint8_t x);  
  void parse_gpgga(char *gpgga);
};


#endif
