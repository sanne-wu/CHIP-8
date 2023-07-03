CC = g++
CONSERVATIVE_FLAGS = -std=c++11 -Wall -Wextra -pedantic
DEBUGGING_FLAGS = -g -O0
CFLAGS = $(CONSERVATIVE_FLAGS) $(DEBUGGING_FLAGS)

main: main.o chip8.o
	$(CC) -lSDL2 -o main main.o chip8.o

main.o: main.cpp chip8.h
	$(CC) $(CFLAGS) -c main.cpp

chip8.o: chip8.cpp chip8.h
	$(CC) $(CFLAGS) -c Chip8.cpp

clean:
	rm -f *.o main
