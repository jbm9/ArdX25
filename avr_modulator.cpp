#include <avr/io.h>
#include <avr/interrupt.h>
#include "avr_modulator.h"

// Lifted from the Arduino/Wiring.h file.
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#include "Arduino.h"
#include "HardwareSerial.h"
extern HardwareSerial Serial;

AVRModulator modulator = AVRModulator();

#define ENABLE_TONE_CLK  bitWrite(TIMSK1, OCIE1A, 1); bitWrite(PORTB, 5, 1);
#define DISABLE_TONE_CLK bitWrite(TIMSK1, OCIE1A, 0); bitWrite(PORTB, 5, 0);


AVRModulator::AVRModulator() {
  ax25e = AX25Encoder();
  tx_dcd_lockout = false;
}


void AVRModulator::setup_hardware() {
  // Turn off timer1 interrupts while we twiddle
  DISABLE_TONE_CLK;
 
  Serial.println("  Setting up Timer1");

  // Clear state
  TCCR1A = 0x00;    // See S15.11 Register descriptions p135-137
  TCCR1B = 0x00;    // p137-138
  TCCR1C = 0x00;    // p138-139

  // Go to CTC, p137
  bitWrite(TCCR1B, WGM13, 0);
  bitWrite(TCCR1B, WGM12, 1);
  bitWrite(TCCR1A, WGM11, 0);
  bitWrite(TCCR1A, WGM10, 0);

  // p138
  bitWrite (TCCR1B, CS12, 0);
  bitWrite (TCCR1B, CS11, 1);
  bitWrite (TCCR1B, CS10, 1);

  // Enable output on B1/Digital9, OCR1A
  bitWrite(DDRB, 1, 1);

  // End state: ready to generate tones, but not enabled
}



void AVRModulator::enqueue(uint8_t *buf, uint16_t buflen) {
  ax25e.enqueue(buf, buflen);      
}


#define PHASE_SPEED_MARK  101
#define PHASE_SPEED_SPACE  56


void AVRModulator::setState(uint8_t v) {
  if (v != 0 && !bitRead(TIMSK1, OCIE1A))
    ENABLE_TONE_CLK;

  if (v == XXXSTATE_MARK) {
    OCR1A = PHASE_SPEED_MARK;
  } else if (v == XXXSTATE_SPACE) {
    OCR1A = PHASE_SPEED_SPACE;
  } else if (v == 0) {
    DISABLE_TONE_CLK;
  }
}

bool AVRModulator::nextBit() {
  uint8_t v = ax25e.nextState();
  setState(v);
}


ISR(TIMER1_COMPA_vect) {
  PORTB ^= 1<<1;
}


