.PHONY: all

all: Battleships a.out

Battleships: Main.cpp ConnManager.h ConnManager.cpp Client.h Client.cpp Game/Battleship.h Game/NewPlayer.h Game/NewPlayer.cpp Game/Grid.h
	  g++ -o Battleships Main.cpp ConnManager.cpp Client.cpp Game/NewPlayer.cpp

a.out: Main.cpp ConnManager.h ConnManager.cpp Client.h Client.cpp
	   g++ -g Main.cpp ConnManager.cpp Client.cpp Game/NewPlayer.cpp