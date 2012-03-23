#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define GPSHANDLER_BUFLEN 128

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
