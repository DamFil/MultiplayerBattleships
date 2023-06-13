.PHONY: all

all: test a.out

test: Game.cpp NewPlayer.h NewPlayer.cpp Grid.h Battleship.h
	  g++ Game.cpp NewPlayer.cpp -o test

a.out: Game.cpp NewPlayer.h NewPlayer.cpp Grid.h Battleship.h
	   g++ -g Game.cpp NewPlayer.cpp