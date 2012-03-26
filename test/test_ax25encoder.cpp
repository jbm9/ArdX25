#include <string.h>
#include <stdio.h>
#include <unittest++/UnitTest++.h>


#include "ax25encoder.h"


SUITE(AX25Encoder) {
  struct AX25EncoderFixture {
    AX25Encoder ax25e;

    AX25EncoderFixture() { 
      ax25e = AX25Encoder();
    }

  };


  TEST_FIXTURE(AX25EncoderFixture, TestInitialization) {
    CHECK(!ax25e.inSend());
    CHECK(0 == ax25e.nextState());
  }


  TEST_FIXTURE(AX25EncoderFixture, TestBits) {
    uint8_t bitpos = 0;
    uint8_t bytepos = 0;

    char outbuf[1024];

    int curstate = XXXSTATE_MARK;

    bool keepgoing = true;


    const char *expected = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \xbe\x7c\xf9\xf2\xe5\xcb\x97\x2f\x5f ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    memset(outbuf, 0, 1024);
    const char * msg = " ~~~~~~~~ ";
    ax25e.enqueue((uint8_t*)msg, strlen(msg));

    while (keepgoing) {
      int nextstate = ax25e.nextState();
      printf("%d/%d: %d<>%d: %d ==> %x\n", bytepos, bitpos, curstate, nextstate, nextstate != curstate ? 0 : 1, outbuf[bytepos]);
      if (nextstate == 0) {
	keepgoing = false;
      } else {
	if (curstate == nextstate) {
	  outbuf[bytepos] |= 1 << bitpos;
	}
	bitpos++;
	if (8 == bitpos) {
	  printf("DECODE: %i: %x / %x\n", bytepos, expected[bytepos], outbuf[bytepos]);
	  bytepos++;
	  bitpos = 0;
	}

      }
      curstate = nextstate;
    }
    
    CHECK(bitpos == 0);
    CHECK(bytepos == strlen(expected));
    CHECK(0 == strncmp(expected, outbuf, strlen(expected)));

    printf("hi mom\n");

  }
}

int main(int argc, char *argv[]) {
  return UnitTest::RunAllTests();
}
