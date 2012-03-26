/*
 *
 * ax25encoder.cpp: AX.25 bitstream encoder
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

#include "ax25encoder.h"

#define INITIAL_PHASE_STATE true

// @export class_definition
AX25Encoder::AX25Encoder() {
  phase_state = INITIAL_PHASE_STATE;
  buf = 0;
  buflen = 0;
  bufpos = 0;
  bitpos = 0;
  
  n_ones = 0;
  flagrem = 0;
};


// @export "foo"

#define HEAD_FLAGS 100
#define TAIL_FLAGS 50

#define XXXSTATE_MARK 1
#define XXXSTATE_SPACE 2

// @export "bar"

// @export enqueue
/**
 * Set up the given buffer as our output buffer.  Not so much
 * "enqueue" as "output."
 *
 * @param[in] inbuf Buffer containing the bytes to send
 * @param[in] blen The number of bytes that need to be sent
 *
 * @returns Void.
 */
void AX25Encoder::enqueue(uint8_t *inbuf, uint16_t blen) {
  buf = inbuf;
  bufpos = 0;
  buflen = blen;
  bitpos = 0;
  flagrem = HEAD_FLAGS;
  phase_state = INITIAL_PHASE_STATE;
}


/**
 * Check if we're still in-progress on a message send.
 *
 * @returns True if bits still need to be sent, false otherwise.
 */
bool AX25Encoder::inSend() {
  return !(bufpos == buflen && flagrem == 0);
}

// @export nextstate
/**
 * Get the next bit to send

 * This function implements the AX.25 state machine, including the
 * flags to send at the beginning and end of the message, and the
 * bit-stuffing required when sending more than 5 1s in a row.
 *
 * @returns A number returning the next modulator state (0 is off, 1 is mark, 2 is space)
 */
uint8_t AX25Encoder::nextState() {
  uint8_t curbit;

  if (bufpos == buflen && flagrem == 0) return 0;

  if (flagrem) {
    curbit = 0x7E & (1<<bitpos);

    if (bitpos == 7) {
      bitpos = 0;
      flagrem--;
    } else {
      bitpos++;
    }
	
  } else {

    if (n_ones == 5) { // bit-stuffing: engage.
      curbit = 0;
    } else {
      curbit = buf[bufpos] & (1<<bitpos);
	  
      if (bitpos == 7) {
	bitpos = 0;
	bufpos++;

	if (bufpos == buflen) {
	  flagrem = TAIL_FLAGS;
	}
      } else {
	bitpos++;
      }
    }
  }

  if (!curbit) {
    phase_state = !phase_state;
    n_ones = 0;
  } else {
    n_ones++;
  }

  return phase_state ? XXXSTATE_MARK : XXXSTATE_SPACE;
}
