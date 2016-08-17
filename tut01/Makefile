CC = g++
LD = g++
CFLAGS = -c -I/usr/include/SDL2
LDFLAGS = -lSDL2 -lGLEW -lGL

OBJS = triangle.o

all: triangle

triangle: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o triangle

triangle.o: triangle.cpp
	$(CC) $(CFLAGS) triangle.cpp

clean:
	rm -f *.o triangle

.PHONY: all clean
