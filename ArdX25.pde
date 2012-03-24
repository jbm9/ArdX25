#include <avr/io.h>
#include <util/delay.h>
#include "ax25encoder.h"
#include "avr_modulator.h"
#include "ax25decoder.h"
#include "aprs_weather.h"
#include "gps_handler.h"

#include "Arduino.h"

#include "FCS.h"

#define DESTINATION "APZ1110"
#define CALLSIGN "AJ9BM ;"
#define REPEATER "WIDE2 2"

/**
 * hdlc_frame -- Prepare an AX.25 frame for transmission.
 *

 * This takes the string pointed at by inbuf, of length inlen, and
 * adds the AX.25 header with your callsign, etc, and checksum.  Call
 * this before calling start_xmit();
 *
 * For all practical intents and purposes, outbuf should be hard-coded
 * to xmit (a global in Buffer State), but this structure makes
 * testing the thing way easier.
 *
 * NB: xmit is teensy!  Keep that in mind when crafting your packets.
 *
 * This returns the final size of the output buffer, which you'll want
 * to keep to pass on start_xmit() below.
 *
 * A typical call sequence is:
 *   char *packet = ">Hi from space, mom!";
 *   uint16_t packet_len = strlen(packet);
 *   uint16_t frame_len = hdlc_frame(xmit, (const uint8_t*)packet, packet_len);
 *   start_xmit(frame_len);
 *   _delay_ms(15+(frame_len+250)/150); // time for the packet to transmit
 *
 * The delay is optional; just don't try to xmit another packet in
 * that window, or you're going to mangle the current one.
 *
 */
uint16_t hdlc_frame(uint8_t *outbuf, const uint8_t *inbuf, uint16_t inlen) {
  uint16_t pos = 0;
  uint16_t i;

  uint16_t fcs;

  // Add AX.25 frame header; see TODO REFERENCE

  // Our Address field
  memcpy(outbuf+pos, DESTINATION, 7);  // JBM 7
  pos += 7;
  memcpy(outbuf+pos, CALLSIGN, 7);   // JBM 7
  pos += 7;
  memcpy(outbuf+pos, REPEATER, 7);   // JBM7
  pos += 7;

  // Shift everything over for the AX.25 address header.
  for(i = 0 ; i < pos; i++) {
    outbuf[i] <<= 1;
  }
  outbuf[6] |= 0x80; // set the c-bit
  outbuf[pos-1] |= 1;  // mark end of address

  outbuf[pos] = 0x03; // AX.25 UI Frame
  pos += 1;
  outbuf[pos] = 0xF0; // AX.25: No layer 3 encoding
  pos += 1;


  // The actual contents of the packet
  memcpy(outbuf+pos, inbuf, inlen);
  pos += inlen;


  // And our checksum
  fcs = calc_fcs((uint8_t *)outbuf, pos);
  outbuf[pos] = fcs & 0xFF;
  pos += 1;
  outbuf[pos] = (fcs >> 8);
  pos += 1;

  return pos;
}


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



#ifdef ENABLE_DUMPSTATE
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
#else
#define dumpstate(x) {}
#endif




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
  modulator.nextBit();
}


void cycle_leds() {
  Serial.println("foo");
  uint8_t pins[] = { PIN_GPS_RX, PIN_GPS_LOCK, PIN_APRS_DCD, PIN_APRS_XMIT };

  for (uint8_t i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
    digitalWrite(pins[i], 1);
    _delay_ms(100);
  }
  _delay_ms(2000);
  _delay_ms(2000);
  _delay_ms(2000);
  _delay_ms(2000);
  _delay_ms(2000);
  for (uint8_t i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
    digitalWrite(pins[i], 0);
    _delay_ms(100);
  }

}






void setup() {
  Serial.begin(4800);

  delay(100);
  Serial.println("Starting up.");

  dumpstate("STARTUP");

  delay(100);

  bitWrite(DDRD, 6, 1); // D6: APRS_DCD
  bitWrite(PORTD, 6, 1);

  bitWrite(DDRD, 7, 1); // D7: GPS_RX
  bitWrite(DDRB, 0, 1); // D8: GPS_LOCK
  bitWrite(DDRB, 5, 1); // D13: APRS_XMIT

  bitWrite(DDRB, 1, 1); // D9: Analog output
  bitWrite(DDRB, 3, 1); // D11: test/debug
  bitWrite(DDRB, 4, 1); // D12: test/debug

  cycle_leds();
  modulator.setup_hardware();

  modulator.setState(2);
  cycle_leds();

  modulator.setState(1);
  cycle_leds();
  modulator.setState(0);


  rx_state_setup();

  baud_interrupt_setup();

  ENABLE_BAUD_CLK;

  dumpstate("exsetup");
}


#define XMIT_EVERY 2 // how many GPGGAs between transmits
#define BUFLEN 100


GPSHandler gpsh = GPSHandler();


uint8_t gps_rx_lights = 0;

uint16_t lastxmit = 0;

uint16_t gpgga_seen = 0;
#define XMIT_EVERY 6*60+13



void loop() {
  int x = Serial.read();
 
  uint16_t curmillis = millis();

  if (-1 != x) {
    Serial.print((char)x);
    if (gpsh.saw(x & 0xFF)) {

      gpgga_seen = (gpgga_seen + 1) % XMIT_EVERY;

      if (1 == gpgga_seen) {

	Serial.println("==========");
	Serial.println((char*)gpsh.buf);
	Serial.println("==========");

	Serial.println( millis() );

	uint8_t xmit[150];
	uint16_t pos = hdlc_frame(xmit, (const uint8_t *)gpsh.buf, gpsh.bufpos);

	gpsh.bufpos = 0;
	modulator.enqueue(xmit, pos);
      }
    }
    gps_rx_lights = 100;
    _delay_ms(1);
  }

  if (gps_rx_lights) gps_rx_lights--;

  digitalWrite(7, gps_rx_lights);
}
