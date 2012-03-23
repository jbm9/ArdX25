#include "APRSTelemetry.h"

void APRSTelemetry::note_temp_internal(uint8_t t) {
  temp_internal = t;
}

void APRSTelemetry::note_temp_external(uint8_t t) {
  temp_external = t;
}

void APRSTelemetry::note_pressure(uint8_t t) {
  pressure = t;
}

void APRSTelemetry::note_acc(uint8_t, x, uint8_t y, uint8_t z) {
  acc_x = x;
  acc_y = y;
  acc_z = z;
}

// Using an AREF=3.3V, with 8b outputs, convert the reading into
// degrees Fahrenheit, which APRS insists on using.  This formula uses
// integer math only, and yields an accuracy of -1~+3 degF over the
// entire range, with a mean error of -0.018, sd=0.1898.
int8_t APRSTelemetry::temp_to_fahrenheit(uint8_t t) {
  int8_t tp = t - 199;
  return (2*tp + tp/3 + 2);
}

int8_t APRSTelemetry::temp_internal_fahrenheit() {
  return temp_to_fahrenheit(temp_internal);
}

int8_t APRSTelemetry::temp_external_fahrenheit() {
  return temp_to_fahrenheit(temp_external);
}


// APRS101.pdf, Chapter 12 (p65 / PDF page 75)
uint16_t APRSTelemetry::format_string(char *buf, uint16_t maxbuf) {
  uint16_t pos = 0;

  *(buf+pos) = ';';
  pos++;

  char OBJECT_NAME[9] = { 'B', 'a', 'c', 'c', 'h', 'u','s', ' ', '9' }

  memcpy(buf+pos, 9, OBJECT_NAME);
  pos += 9;

  *(buf+pos) = '*';
  pos++;
      

  snprintf(buf+pos, 7, '%04d00z', gpsh->hhmm);
  pos += 7;

  snprintf(buf+pos, 5, '%04d.', abs(gpsh->lat_whole));
  pos += 5;
  snprintf(buf+pos, 2, '%02d', gpsh->lat_frac);
  pos += 2;
  *(buf+pos) = (gpsh->lat_whole > 0 ? 'N' : 'S');
  pos++;

  *(buf+pos) = '/'; // TODO CHOOSE SYMBOL
  pos++;

  snprintf(buf+pos, 5, '%04d.', abs(gpsh->lon_whole));
  pos += 5;
  snprintf(buf+pos, 2, '%02d', gpsh->lon_frac);
  pos += 2;
  *(buf+pos) = (gpsh->lon_whole > 0 ? 'E' : 'W');
  pos++;

  *(buf+pos) = '_'; // TODO CHOOSE SYMBOL
  pos++;

  // Wind speed, which we don't have.
  for (uint8_t i = 0; i < 7; i++) {
    *(buf+pos) = '.';
    pos++;
  }
    
  // Pack up Weather data

  *(buf+pos) = 'g';
  pos++;
  for (uint8_t i = 0; i < 3; i++) {
    *(buf+pos) = '.';
    pos++;
  }

  *(buf+pos) = 't';
  pos++;
  // External temperature
  snprintf(buf+pos, 3, "%03d", temp_external_fahrenheit());
  pos += 3;

  *(buf+pos) = 'b';
  pos ++;
  // XXX TODO Convert pressure into a value here.
  pos += 5;


  *(buf+pos) = 'b'; // APRS Software Type
  pos++;
  memcpy(buf+pos, 4, 'ccs9');
  pos += 4;    

  return pos;    
}


