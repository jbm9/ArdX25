/*
 *
 * FCS.h: Frame Check Sum implementation interface.
 * 
 * This implements a checksum algorithm used by the HDLC framing
 * for AFSK.
 *
 *  This is based primarily on code in the public domain; feel free to
 *  consider this file in the public domain as well.
 *
 *******************************************************************/


#ifndef _FCS_H_
#define _FCS_H_

#include <stdint.h>

#include "Arduino.h"
#include <avr/pgmspace.h>

extern "C" {

extern uint16_t calc_fcs(uint8_t const *buf, uint16_t cnt);

}


#endif /* ndef _FCS_H */
