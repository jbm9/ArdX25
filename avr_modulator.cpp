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



#define USE_CLOSE_SINE 1
// #define JBM_USE_PWM 1 // PWM is broken at the moment.


#ifdef JBM_USE_PWM
#define ENABLE_TONE_CLK  bitWrite(TCCR1A, COM1A1, 1); bitWrite(TCCR1A, COM1A0, 0); bitWrite(TIMSK1, TOIE1, 1); bitWrite(PORTB, 5, 1);
#define DISABLE_TONE_CLK bitWrite(TCCR1A, COM1A1, 0); bitWrite(TCCR1A, COM1A0, 0); bitWrite(TIMSK1, TOIE1, 0); bitWrite(PORTB, 5, 0);
#else
#define ENABLE_TONE_CLK  bitWrite(TIMSK1, OCIE1A, 1); bitWrite(PORTB, 5, 1);
#define DISABLE_TONE_CLK bitWrite(TIMSK1, OCIE1A, 0); bitWrite(PORTB, 5, 0);
#endif



AVRModulator::AVRModulator() {
  ax25e = AX25Encoder();
  setup();
}


void AVRModulator::setup_hardware() {
  // Turn off timer1 interrupts while we twiddle
  DISABLE_TONE_CLK;
 
  Serial.println("  Setting up Timer1");

  // Clear state
  TCCR1A = 0x00;    // See S15.11 Register descriptions p135-137
  TCCR1B = 0x00;    // p137-138
  TCCR1C = 0x00;    // p138-139

#ifdef JBM_USE_PWM
  // Go to fast PWM 8b, p137
  bitWrite(TCCR1B, WGM12, 0);
  bitWrite(TCCR1A, WGM11, 1);
  bitWrite(TCCR1A, WGM10, 0);


  // Turn on a div-8 prescaler/divider: p138
  bitWrite (TCCR1B, CS12, 0);
  bitWrite (TCCR1B, CS11, 1);
  bitWrite (TCCR1B, CS10, 0);
#else
  // Go to CTC, p137
  bitWrite(TCCR1B, WGM13, 0);
  bitWrite(TCCR1B, WGM12, 1);
  bitWrite(TCCR1A, WGM11, 0);
  bitWrite(TCCR1A, WGM10, 0);

  bitWrite (TCCR1B, CS12, 1);
  bitWrite (TCCR1B, CS11, 0);
  bitWrite (TCCR1B, CS10, 1);
#endif
  // Enable output on B1/Digital9, OCR1A
  bitWrite(DDRB, 1, 1);

  // End state: ready to generate tones, but not enabled
}

void AVRModulator::setup() {
  curv = 0;

  sine_i = 1;
  sine_count = 0;

  tx_dcd_lockout = false;
}

void AVRModulator::enqueue(uint8_t *buf, uint16_t buflen) {
  ax25e.enqueue(buf, buflen);      
}

#ifdef JBM_USE_PWM
#define PHASE_SPEED_MARK  4
#define PHASE_SPEED_SPACE 3
#else
#define PHASE_SPEED_MARK  25*4+2 // 1.20195kHz
#define PHASE_SPEED_SPACE 13*4+3 // 2.1930kHz
#endif

void AVRModulator::setState(uint8_t v) {
  if (v != 0 && curv == 0 && !bitRead(TIMSK1, TOIE1))
    ENABLE_TONE_CLK;

  fudge = fudge ? 0 : 1;

  if (v == XXXSTATE_MARK) {
    curv = PHASE_SPEED_MARK+fudge;
  } else if (v == XXXSTATE_SPACE) {
    curv = PHASE_SPEED_SPACE+fudge;
  } else if (v == 0) {
    curv = 0;
    DISABLE_TONE_CLK;
    return;
  }

#ifndef JBM_USE_PWM
   OCR1A = curv;
#endif

}

bool AVRModulator::nextBit() {
  uint8_t v = ax25e.nextState();
  setState(v);
}



#ifdef JBM_USE_PWM

ISR(TIMER1_OVF_vect) { // At div8, this happens at 7812.5Hz, every 2k clocks
#if USE_CLOSE_SINE
  static const uint8_t sine[16] = { 7, 10, 13, 14, 15, 14, 13, 10, 
				  8,  5,  2,  1,  0,  1,  2,  5, };
#else
// This is a better fit with "sag" accounted for.
  static const uint8_t sine[16] = { 8, 11, 13, 14, 15, 14, 13, 11, 
				  9,  6,  3,  2,  1,  2,  3,  6, };
#endif

  OCR1A = sine[modulator.sine_i] << 4;
  modulator.sine_i = (modulator.sine_i + modulator.curv)%16;

}

#else

ISR(TIMER1_COMPA_vect) {
  PORTB ^= 1<<1;
  TCNT1 = 0;
}


#endif
