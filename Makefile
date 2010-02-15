# The Makefile
#
# If you're using Windows and you don't know what this file is,
# don't worry about it. Just use Visual C++ Express Edition or
# Dev-C++ to work on your code.

all: MyTronBot

MyTronBot: MyTronBot.o Map.o
	g++ -Wall -Wextra -o MyTronBot -g MyTronBot.o Map.o

MyTronBot.o: MyTronBot.cc Map.h
	g++ -Wall -Wextra -c -g -O2 -o MyTronBot.o MyTronBot.cc

Map.o: Map.cc Map.h
	g++ -Wall -Wextra -c -g -O2 -o Map.o Map.cc

clean:
	rm -f *.o MyTronBot

.PHONY: clean
