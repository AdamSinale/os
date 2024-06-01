CC = g++
FLAGS = -Wall -g -Wextra

all: ex1_1 ex1_2 ex1_3

ex1_1: ex1_1.cpp
	$(CC) $(FLAGS) -o $@ $^
ex1_2: ex1_2.cpp
	$(CC) $(FLAGS) -o $@ $^
ex1_3: ex1_3.cpp
	$(CC) $(FLAGS) -o $@ $^
	

.PHONY: clean
clean:
	rm -f *.o ex1_1 ex1_2 ex1_3
