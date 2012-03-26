/*
 *
 * gps_handler.cpp: DESCRIPTION
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

#include "gps_handler.h"

GPSHandler::GPSHandler() {
  memcpy(expression, "$GPGGA", 6);
  bufpos = 0;
  state = 0;
  lock = false;

  hhmm = -1;
  lat_whole = 500;
  lat_frac = -1;

  lon_whole = 500;
  lon_frac = -1;

  altitude = -99999;
}


/**
 * Submit the given character as a "seen" byte in the serial stream.
 *
 * This module is a little inside out: it accumulates the GPS
 * sentences itself, in its buffer, then returns true from here when a
 * complete GPGGA sentence is in the buf variable.
 *
 * @param[in] x The latest byte read from serial
 *
 * @returns True if there's a GPGGA sentence in this.buf, false otherwise.
 */
bool GPSHandler::saw(uint8_t x) {
  bool retval = false;

  if (state < 6) {
    if (x == expression[state]) {
      state++;
      buf[bufpos] = x;
      bufpos++;
    } else {
      state = 0;
      bufpos = 0;
    }
    return false;
  }
  
  if (bufpos >= GPSHANDLER_BUFLEN-1 || '$' == x || '\n' == x) { // got terminus
    if (bufpos > 6) {
      buf[bufpos] = '\0';
      parse_gpgga((char *)buf);
      retval = true;
    }

    state = ('$' == x); // 0 on newline, 1 on $
  } else {
    buf[bufpos] = x;
    bufpos++;
  }
  return retval;
}


/**
 * Extracts the values we care about from the given GPGGA sentence.
 *
 * Note that this method doesn't check the checksum or anything else
 * helpful; it simply parses the data down to the values specified.
 *
 * @param[in] gpgga A NUL-terminated buffer containing our $GPGGA
 * sentence.
 */
void GPSHandler::parse_gpgga(char *gpgga) {
  uint16_t new_hhmm;
  uint16_t new_lat_whole;
  uint8_t  new_lat_frac;
  uint16_t new_lon_whole;
  uint8_t new_lon_frac;
  uint32_t new_altitude;

  char q;
    
  if (memcmp(gpgga, "$GPGGA", strlen("GPGGA"))) {
    return;
  }

  // $GPGGA,021741,3745.5748,N,12224.9648,W,2,09,1.0,20.2,M,-32.1,M,,*48

#define ADVANCE_PAST(ptr,c) { while ((*(ptr)) != (c) && *(ptr)) ptr++; if (!*ptr) return; ptr++;}

  ADVANCE_PAST(gpgga, ','); // $GPGGA,


  // Extract the hour/minute timestamp; crap all over the seconds'
  // tens place to truncate atoi's return value to a manageable 16b
  q = *(gpgga+4);
  
  *(gpgga+4) = '|';
  new_hhmm = atoi(gpgga);
  *(gpgga+4) = q;
  ADVANCE_PAST(gpgga, ','); // timestamp

  // Extract latitude, with sign
  new_lat_whole = atoi(gpgga);
  ADVANCE_PAST(gpgga, '.');
  q = *(gpgga+2);
  *(gpgga+2) = '\0';
  new_lat_frac = atoi(gpgga);
  *(gpgga+2) = q;
  ADVANCE_PAST(gpgga, ',');
  if (*gpgga == 'S') new_lat_whole *= -1;
  ADVANCE_PAST(gpgga, ',');


  // Extract longitude, with sign
  new_lon_whole = atoi(gpgga);
  ADVANCE_PAST(gpgga, '.');
  q = *(gpgga+2);
  *(gpgga+2) = '\0';
  new_lon_frac = atoi(gpgga);
  *(gpgga+2) = q;
  ADVANCE_PAST(gpgga, ',');
  if (*gpgga == 'W') new_lon_whole *= -1;
  ADVANCE_PAST(gpgga, ',');

  // Fix information
  lock = (*gpgga != '0');

  ADVANCE_PAST(gpgga, ',');
  // now at number of satellites; skipped.
  // number_of_satellites = atoi(gpgga);

  ADVANCE_PAST(gpgga, ',');
  // horizontal dilution of precision stat

  ADVANCE_PAST(gpgga, ',');
  // altitude
  new_altitude = atoi(gpgga); // XXX Ignores fractions

  ADVANCE_PAST(gpgga, ','); // Now at the 'units' field, should be 'M'
  if (*gpgga != 'M') return;

  // Don't care about the remainder of the message.


  // commit the new position in a single transaction
  lat_whole = new_lat_whole;
  lat_frac = new_lat_frac;
  lon_whole = new_lon_whole;
  lon_frac = new_lon_frac;
  altitude = new_altitude;
  hhmm = new_hhmm;

#undef ADVANCE_PAST   
}

