#ifndef APRS_WEATHER_H
#define APRS_WEATHER_H

#include "gps_handler.h"

class APRSWeather {
public:
  uint8_t temp_internal, temp_external;
  uint8_t pressure;
  uint8_t acc_x, acc_y, acc_z;


  GPSHandler *gpsh;

  void set_gps_handler(GPSHandler *g);

  void note_temp_internal(uint8_t t);
  void note_temp_external(uint8_t t);
  void note_pressure(uint8_t t);
  void note_acc(uint8_t x, uint8_t y, uint8_t z);

  int8_t temp_to_fahrenheit(uint8_t t);
  int8_t temp_internal_fahrenheit();
  int8_t temp_external_fahrenheit();

  void note_gpgga(char *gpgga);
  uint16_t format_string(char *buf, uint16_t maxbuf);

};

#endif
