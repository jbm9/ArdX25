#include <util/delay.h>
#include "ax25encoder.h"
#include "avr_modulator.h"
#include "ax25decoder.h"

typedef struct rx_state {
  AX25Decoder ax25d; // Actual decoder state

  uint16_t last_xc; // timestamp of last zero-crossing
  uint8_t obs_i;    // dt observation cursor
  uint8_t obs[5];   // list of dts observed 

  uint8_t buf[300]; // text output buffer
  uint16_t bufpos;  // text output buffer cursor

} rx_state_t;

rx_state_t rx_state;

void rx_state_setup() {
  rx_state.ax25d = AX25Decoder();
  rx_state.obs_i = 0;

  rx_state.bufpos = 0;
}

#define PIN_GPS_RX 7
#define PIN_GPS_LOCK 8
#define PIN_APRS_DCD 6
#define PIN_APRS_XMIT 13

extern AVRModulator modulator; // = AVRModulator();


#define ENABLE_BAUD_CLK  bitWrite(TIMSK2, OCIE2A, 1);
#define DISABLE_BAUD_CLK bitWrite(TIMSK2, OCIE2A, 0);


// RX also hooks onto the baud interrupt.
void baud_interrupt_setup() {
  DISABLE_BAUD_CLK;

  Serial.println("  Setting up Timer2");

  // Clear out settings.
  TCCR2A = 0x00;    // See S17.11 Register Description, pp159-61
  TCCR2B = 0x00;    // pp162-3

  // Turn on a div-1024 prescaler/divider: p163
  bitWrite (TCCR2B, CS22, 1);
  bitWrite (TCCR2B, CS21, 1);
  bitWrite (TCCR2B, CS20, 1);


  // Set waveform mode to something that resets at OCR2A p161

  // Currently using 2, CTC
  bitWrite(TCCR2B, WGM22, 0);
  bitWrite(TCCR2A, WGM21, 1);
  bitWrite(TCCR2A, WGM20, 0);
 
  // Set an initial, conservative speed.
  OCR2A = 13-1; // 1201.923 Hz

  // This should be ready to go now.
}




ISR(TIMER2_COMPA_vect) { // baud clock
  // modulator.nextBit();
}



void dumpstate(char *name) {
  Serial.print("======= ");
  Serial.println(name);

  Serial.print("OCR1A=");
  Serial.println(OCR1A,10);

  Serial.print("TCCR1A=");
  Serial.println(TCCR1A, 16);
  Serial.print("TCCR1B=");
  Serial.println(TCCR1B, 16);
  Serial.print("TCCR1C=");
  Serial.println(TCCR1C, 16);

}


void setup() {
  Serial.begin(57600);

  delay(100);
  Serial.println("Starting up.");

  dumpstate("STARTUP");

  delay(100);

  bitWrite(DDRD, 6, 1); // D6: APRS_DCD
  bitWrite(DDRD, 7, 1); // D7: GPS_RX
  bitWrite(DDRB, 0, 1); // D8: GPS_LOCK
  bitWrite(DDRB, 5, 1); // D13: APRS_XMIT

  bitWrite(DDRB, 1, 1); // D9: Analog output
  bitWrite(DDRB, 3, 1); // D11: test/debug
  bitWrite(DDRB, 4, 1); // D12: test/debug

  modulator.setup_hardware();
  /*
  cycle_leds();
  cycle_leds();
  */


  rx_state_setup();

  baud_interrupt_setup();

  ENABLE_BAUD_CLK;

  dumpstate("exsetup");
}

#define XMIT_EVERY 37 // how many GPGGAs between transmits

void cycle_leds() {
  Serial.println("foo");
  uint8_t pins[] = { PIN_GPS_RX, PIN_GPS_LOCK, PIN_APRS_DCD, PIN_APRS_XMIT };

  for (uint8_t i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
    digitalWrite(pins[i], 1);
    _delay_ms(100);
  }
  _delay_ms(2000);
  for (uint8_t i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
    digitalWrite(pins[i], 0);
    _delay_ms(100);
  }

}


void loop() {
  dumpstate("loop0");
  digitalWrite(PIN_APRS_DCD, 1);
  modulator.setState(XXXSTATE_MARK);
  dumpstate("loop1");

  _delay_ms(1000);
  _delay_ms(1000);
  _delay_ms(1000);

  digitalWrite(PIN_APRS_DCD, 0);

  modulator.setState(0);
   dumpstate("loop2");

  for (uint8_t i = 0; i < 20; i++) {
    digitalWrite(PIN_APRS_DCD, 1);
    _delay_ms(100);
    digitalWrite(PIN_APRS_DCD, 0);
    _delay_ms(100);
  }


  modulator.setState(XXXSTATE_SPACE);
  dumpstate("loop3");
  _delay_ms(1000);
  _delay_ms(1000);
  _delay_ms(1000);
}
