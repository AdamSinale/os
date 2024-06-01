CC = g++
FLAGS = -Wall -g -Wextra

CPPs=main.cpp Poisson.cpp
OBJECTS=$(subst .cpp,.o,$(CPPs))

Main: main.o $(OBJECTS)
	$(CC) $(FLAGS) $^ -o test

%.o: %.cpp
	$(CC) $(FLAGS) --compile $< -o $@

.PHONY: clean
clean:
	rm -f *.o demo test
