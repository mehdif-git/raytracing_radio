CC = gcc
SAFEFLAGS = -Wall -Wextra -g -pedantic -fsanitize=address -O2
FASTFLAGS = -O3
LDFLAGS = -lm

init: 
	mkdir renders && mkdir models

safe: main_safe

main_safe: geometry.o raytracing.o main.o
	$(CC) $(SAFEFLAGS) $^ -o $@ $(LDFLAGS)

fast: main_fast

main_fast: geometry.o raytracing.o main.o
	$(CC) $(FASTFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

clean:
	rm -f *.o main_safe

