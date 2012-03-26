/*
 *
 * ax25encoder.h: AX.25 bitstream encoder
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

#ifndef AX25ENCODER_H
#define AX25ENCODER_H

#include <inttypes.h>

#define XXXSTATE_MARK  1
#define XXXSTATE_SPACE 2

/**
 * @class AX25Encoder
 * @brief AX.25 bitstream encoder
 *
 * This class encapsulates all the state needed to control a modulator
 * for AX.25.  It doesn't actually modulate, but it can be used to
 * determine the next modulator state.  You should call nextState() at
 * your baud rate (1200Hz for AFSK1200), then set the modulator to the
 * state it returns.
 */
class AX25Encoder {
public:
  AX25Encoder();
  void enqueue(uint8_t*, uint16_t);
  uint8_t nextState();
  bool inSend();

private:
  uint8_t *buf;
  uint16_t buflen;
  uint16_t bufpos;
  uint8_t bitpos;

  uint8_t n_ones;
  uint8_t flagrem;

  bool phase_state;
};

#endif
