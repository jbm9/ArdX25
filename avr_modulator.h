#ifndef AVR_MODULATOR_H
#define AVR_MODULATOR_H

#include "ax25encoder.h"

class AVRModulator {
public:
  AX25Encoder ax25e;

  bool tx_dcd_lockout;

  // ****************************************

  AVRModulator();

  void setup_hardware();
  void enqueue(uint8_t *buf, uint16_t buflen);

  void setState(uint8_t state);
  bool nextBit();
};

#endif
