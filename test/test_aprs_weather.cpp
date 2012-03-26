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
      CHECK(!gpsh.lock);
    }

  TEST_FIXTURE(APRSWeatherFixture, TestBytewiseIngestionSimple)
    {
      const char *gpgga = "$GPGGA,021741,3745.5748,N,12224.9648,W,2,09,1.0,20.2,M,-32.1,M,,*48\n";
      int i;

      for (i = 0; i < strlen(gpgga)-1; i++) {
	CHECK(!gpsh.saw(gpgga[i]));
      }
      CHECK(gpsh.saw(gpgga[i]));
    }

  TEST_FIXTURE(APRSWeatherFixture, TestTemperatures)
    {
#if USE_RAW_3V3_AREF
      aw.note_temp_internal(199);
      aw.note_temp_external(150);
      CHECK(aw.temp_internal_fahrenheit() == 2);
      CHECK(aw.temp_external_fahrenheit() == -112);

      aw.note_temp_internal(252);
      CHECK(aw.temp_internal_fahrenheit() == 125);

      aw.note_temp_internal(255);
      CHECK(aw.temp_internal_fahrenheit() == 125);

      aw.note_temp_external(231);
      CHECK(aw.temp_external_fahrenheit() == 76);
#else
      aw.note_temp_internal(199);
      aw.note_temp_external(150);
      CHECK(aw.temp_internal_fahrenheit() == -18);
      CHECK(aw.temp_external_fahrenheit() == -126);

      aw.note_temp_internal(252);
      CHECK(aw.temp_internal_fahrenheit() == 98);

      aw.note_temp_internal(255);
      CHECK(aw.temp_internal_fahrenheit() == 104);

      aw.note_temp_external(231);
      CHECK(aw.temp_external_fahrenheit() == 51);
#endif
    }


  TEST_FIXTURE(APRSWeatherFixture, TestPressure) {
    aw.note_pressure(140);
    CHECK(aw.pressure_millibars() == 700);
    aw.note_pressure(219);
    CHECK(aw.pressure_millibars() == 1040);
    
  }


  TEST_FIXTURE(APRSWeatherFixture, TestFormatString)
    {
      char buf[1024];
      uint16_t pos;


      const char *gpgga = "$GPGGA,021741,3745.5748,N,12224.9648,W,2,09,1.0,20.2,M,-32.1,M,,*48\n";
      int i;

      for (i = 0; i < strlen(gpgga)-1; i++) {
	CHECK(!gpsh.saw(gpgga[i]));
      }
      CHECK(gpsh.saw(gpgga[i]));


      aw.note_temp_external(231);
      aw.note_pressure(140);

      pos = aw.format_string(buf, 1024);

      const char *expected_packet = ";AJ9BM-11 *021700h3745.57N/12224.96W_.../...t051b07000";
      const uint16_t expected_len = strlen(expected_packet);

      CHECK(pos == expected_len);
      CHECK(0 == strncmp(expected_packet, buf, expected_len));
    }

}

int main(int argc, char *argv[]) {
  return UnitTest::RunAllTests();
}
