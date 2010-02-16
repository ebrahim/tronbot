# The Makefile
#
# If you're using Windows and you don't know what this file is,
# don't worry about it. Just use Visual C++ Express Edition or
# Dev-C++ to work on your code.

CXXFLAGS=-Wall -Wextra -Wno-deprecated -O2 -g #-pg

all: MyTronBot

MyTronBot: MyTronBot.o
	g++ $(CXXFLAGS) -o MyTronBot MyTronBot.o

MyTronBot.o: MyTronBot.cc Map.h
	g++ $(CXXFLAGS) -c -o MyTronBot.o MyTronBot.cc

clean:
	rm -f *.o MyTronBot

.PHONY: clean
