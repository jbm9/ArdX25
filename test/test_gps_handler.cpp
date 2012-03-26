#include <stdio.h>
#include <unittest++/UnitTest++.h>


#include "gps_handler.h"


SUITE(GPSHandler) {


  struct GPSHandlerFixture {
    GPSHandler gpsh;

    GPSHandlerFixture() { 
      gpsh = GPSHandler();
    }

  };


  TEST_FIXTURE(GPSHandlerFixture, TestInitialization) {
    CHECK(!gpsh.lock);
  }


  TEST_FIXTURE(GPSHandlerFixture, TestBytewiseIngestionSimple) {
    const char *gpgga = "$GPGGA,021741,3745.5748,N,12224.9648,W,2,09,1.0,20.2,M,-32.1,M,,*48\n";
    unsigned int i;

    for (i = 0; i < strlen(gpgga)-1; i++) {
      CHECK(!gpsh.saw(gpgga[i]));
    }
    CHECK(gpsh.saw(gpgga[i]));
  }


  TEST_FIXTURE(GPSHandlerFixture, TestIngestionRecovery) {
    const char *gpgga = "$GP this is not gps whee \002\077@!#$%&$$$GPGGA,021741,3745.5748,N,12224.9648,W,2,09,1.0,20.2,M,-32.1,M,,*48\n";
    unsigned int i;

    for (i = 0; i < strlen(gpgga)-1; i++) {
      CHECK(!gpsh.saw(gpgga[i]));
    }
    CHECK(gpsh.saw(gpgga[i]));
  }

  
  TEST_FIXTURE(GPSHandlerFixture, TestParse) {
    const char *gpgga = "$GP this is not gps whee \002\077@!#$%&$$$GPGGA,081741,3745.5748,N,12224.9648,W,2,09,1.0,20.2,M,-32.1,M,,*48\n";
    unsigned int i;

    for (i = 0; i < strlen(gpgga)-1; i++) {
      CHECK(!gpsh.saw(gpgga[i]));
    }
    CHECK(gpsh.saw(gpgga[i]));

    CHECK(gpsh.hhmm == 817);
    CHECK(gpsh.lat_whole == 3745);
    CHECK(gpsh.lat_frac == 57);
    CHECK(gpsh.lon_whole == -12224);
    CHECK(gpsh.lon_frac == 96);

    CHECK(gpsh.lock);

    CHECK(gpsh.altitude == 20);

  }

}

int main(int argc, char *argv[]) {
  return UnitTest::RunAllTests();
}
