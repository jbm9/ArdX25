#include "gps_handler.h"

GPSHandler::GPSHandler() {
  memcpy(expression, "$GPGGA", 6);
  bufpos = 0;
  rx_count = 0;
  state = 0;
}

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
  
  if (bufpos >= BUFLEN-1 || '$' == x || '\n' == x) { // got terminus
    if (bufpos > 6) {
      buf[bufpos] = '\0';
      parse_gpgga(buf);
      retval = true;
    }

    state = ('$' == x); // 0 on newline, 1 on $
  } else {
    buf[bufpos] = x;
    bufpos++;
  }
  return retval;
}




// Note the position associated with a GPGGA statement; doesn't
// check checksums at all.
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
  *(gpgga+4) = '\0';
  hhmm = atoi(gpgga);
  *(gpgga+4) = q; 
  ADVANCE_PAST(gpgga, ','); // timestamp

  // Extract latitude, with sign
  new_lat_whole = atoi(gpgga);
  ADVANCE_PAST(gpgga, '.');
  q = *(gpgga+2);
  *(gpgga+2) = '\0';
  new_lat_frac = atoi(gpgga);
  gpgga+2 = q;
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
  if (*gpgga == 'W') new_lat_whole *= -1;
  ADVANCE_PAST(gpgga, ',');

  // Fix information
  if (*gpgga == '0') return; // skip non-fix positions for now

  ADVANCE_PAST(gpgga, ',');
  // now at number of satellites; skipped.
  // number_of_satellites = atoi(gpgga);

  ADVANCE_PAST(gpgga, ',');
  // horizontal dilution of precision stat

  ADVANCE_PAST(gpgga, ',');
  // altitude
  new_altitude = atoi(gpgga); // XXX Ignores fractions

  ADVANCE_PAST(','); // Now at the 'units' field, should be 'M'
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

