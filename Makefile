CC=g++
CFLAGS=-Wall -O -std=gnu++11
LDFLAGS=-ljack
EXEC=JackSynth

all: $(EXEC)

$(EXEC): SyntheSound.o JackSynth.o
	$(CC) -o JackSynth SyntheSound.o JackSynth.o $(LDFLAGS)
SyntheSound.o: SyntheSound.cpp
	$(CC) -o SyntheSound.o -c SyntheSound.cpp $(CFLAGS)
JackSynth.o: JackSynth.cpp SyntheSound.hpp
	$(CC) -o JackSynth.o -c JackSynth.cpp $(CFLAGS)

clean:
	rm -f *.o core
mrproper: clean
	rm -f JackSynth
