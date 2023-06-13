.PHONY: all

all: test a.out

test: Game.cpp Player.h Player.cpp Grid.h Battleship.h
	  g++ Game.cpp Player.cpp -o test

a.out: Game.cpp Player.h Player.cpp Grid.h Battleship.h
	   g++ -g Game.cpp Player.cpp