CC=g++
CFLAGS=-g
LDFLAGS=-lUnitTest++

TEST_OBJS=ax25decoder.o ax25encoder.o test_ax25decoder.o

test:test_ax25decoder
	./test_ax25decoder


%.cpp: %.h

%.o: %.cpp Makefile
	$(CC) -c $(CFLAGS) $< -o $@

test_ax25decoder:ax25decoder.o ax25encoder.o test_ax25decoder.o
	$(CC) $(CFLAGS)  -o test_ax25decoder $(TEST_OBJS)  $(LDFLAGS)

test_gps_handler:gps_handler.o test_gps_handler.o
	$(CC) $(CFLAGS)  -o test_gps_handler $< test_gps_handler.o  $(LDFLAGS)


doc: *.tex *.cpp *.h .dexy
	dexy
