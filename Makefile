.PHONY: all

all: test a.out

test: Game/Game.cpp Game/NewPlayer.h Game/NewPlayer.cpp Game/Grid.h Game/Battleship.h
	  g++ Game/Game.cpp Game/NewPlayer.cpp -o test

a.out: Game/Game.cpp Game/NewPlayer.h Game/NewPlayer.cpp Game/Grid.h Game/Battleship.h
	   g++ -g Game/Game.cpp Game/NewPlayer.cpp