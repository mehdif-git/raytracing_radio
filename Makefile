CC = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address -O2
LDFLAGS = -lm

all: test

test: bitmap.o geometry.o raytracing.o test.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o test

