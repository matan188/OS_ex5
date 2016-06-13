CC = gcc
RANLIB = ranlib

LIBSRC = emServer.cpp emServer.h emClient.cpp emClient.h

TAR = tar
TARFLAGS = -cvf
TARNAME = ex5.tar
TARSRCS = $(LIBSRC) Makefile README

all: emServer.cpp emClient.cpp Event.cpp
	g++ -Wall -std=c++11 -pthread emServer.cpp Event.cpp -o emServer
	g++ -Wall -std=c++11 -pthread emClient.cpp -o emClient

clean:
	rm *.a *.o *.tar

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)