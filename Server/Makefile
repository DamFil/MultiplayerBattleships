.PHONY: all

all: BattleshipsServer a.out

BattleshipsServer: ConnectionManager.h ConnectionManager.cpp GameState.h NewServer.cpp Player.h Player.cpp Spectator.h Spectator.cpp
	  g++ -o BattleshipsServer ConnectionManager.cpp NewServer.cpp Player.cpp Spectator.cpp -pthread

a.out: ConnectionManager.h ConnectionManager.cpp GameState.h NewServer.cpp Player.h Player.cpp Spectator.h Spectator.cpp
	   g++ -g ConnectionManager.cpp NewServer.cpp Player.cpp Spectator.cpp -pthread