prefix := /usr/local

# The recommended compiler flags for the Raspberry Pi
CXXFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
CPPFLAGS=-Wall -I../
LDFLAGS=-lrf24-bcm -lrf24network
CC=g++
#CCFLAGS=

# define all programs
PROGRAMS = netmonitor
SOURCES = ${PROGRAMS:=.cpp} message.h

all: ${PROGRAMS}

netmonitor: sensorconfig.o netmonitor.o

netmonitor.o: netmonitor.cpp message.h msgqueue.h sensorconfig.h

sensorconfig.o: sensorconfig.h

clean:
	rm -rf $(PROGRAMS)

install: all
	test -d $(prefix) || mkdir $(prefix)
	test -d $(prefix)/bin || mkdir $(prefix)/bin
	for prog in $(PROGRAMS); do \
	  install -m 0755 $$prog $(prefix)/bin; \
	done

.PHONY: install
