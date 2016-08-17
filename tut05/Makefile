CC = g++
LD = g++

# This is for debugging, not meant for speed atm.
CFLAGS = -c -g -I/usr/include/SDL2 -std=c++14 -Wall -Werror -Wextra
CFLAGS += -pedantic-errors

LDFLAGS = -lSDL2 -lGLEW -lGL

OBJS = cube.o shader_utils.o

all: cube

cube: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o cube

cube.o: source/cube.cpp include/shader_utils.h
	$(CC) $(CFLAGS) source/cube.cpp

shader_utils.o: source/shader_utils.cpp
	$(CC) $(CFLAGS) source/shader_utils.cpp

clean:
	rm -f *.o cube

.PHONY: all clean
