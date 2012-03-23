#include <stdio.h>
#include <unittest++/UnitTest++.h>


#include "aprs_weather.h"


SUITE(APRSWeather) {


  struct APRSWeatherFixture {
    GPSHandler gpsh;
    APRSWeather aw;

    APRSWeatherFixture() { 
      gpsh = GPSHandler();
      aw = APRSWeather();
      aw.set_gps_handler(&gpsh);
    }

  };






  TEST_FIXTURE(APRSWeatherFixture, TestInitialization)
    {
      CHECK(!aw.gpsh->lock);
    }

  TEST_FIXTURE(APRSWeatherFixture, TestBytewiseIngestionSimple)
    {
      char *gpgga = "$GPGGA,021741,3745.5748,N,12224.9648,W,2,09,1.0,20.2,M,-32.1,M,,*48\n";
      int i;

      for (i = 0; i < strlen(gpgga)-1; i++) {
	CHECK(!aw.gpsh->saw(gpgga[i]));
      }
      CHECK(aw.gpsh->saw(gpgga[i]));
    }

}

int main(int argc, char *argv[]) {
  return UnitTest::RunAllTests();
}
