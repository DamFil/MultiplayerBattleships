.PHONY: all

all: Battleships a.out

Battleships: Main.cpp ConnManager.h ConnManager.cpp Client.h Client.cpp 
	  g++ -o Battleships Main.cpp ConnManager.cpp Client.cpp

a.out: Main.cpp ConnManager.h ConnManager.cpp Client.h Client.cpp
	   g++ -g Main.cpp ConnManager.cpp Client.cpp