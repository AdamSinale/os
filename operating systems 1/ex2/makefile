CC = g++
FLAGS = -Wall -g -Wextra

all: Poisson

Poisson: Poisson.cpp
	$(CC) $(FLAGS) -o $@ $^


.PHONY: clean
clean:
	rm -f *.o Poisson
