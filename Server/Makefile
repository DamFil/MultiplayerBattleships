.PHONY: all

all: BattleshipsServer a.out

BattleshipsServer: ConnectionManager.h ConnectionManager.cpp GameState.h NewServer.cpp Player.h Player.cpp
	  g++ -o BattleshipsServer ConnectionManager.cpp NewServer.cpp Player.cpp -pthread

a.out: ConnectionManager.h ConnectionManager.cpp GameState.h NewServer.cpp Player.h Player.cpp
	   g++ -g ConnectionManager.cpp NewServer.cpp Player.cpp -pthread