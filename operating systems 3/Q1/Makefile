
all : Kosaraju

Kosaraju : Kosaraju.o
	g++ -o Kosaraju Kosaraju.o

Kosaraju.o : Kosaraju.cpp
	g++ -c Kosaraju.cpp

clean :	
	rm Kosaraju Kosaraju.o

.PHONY : all clean