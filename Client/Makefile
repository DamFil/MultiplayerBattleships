.PHONY: all

all: Battleships BattleshipsTest a.out

Battleships: Main.cpp ConnManager.h ConnManager.cpp Client.h Client.cpp ../Game/Battleship.h ../Game/NewPlayer.h ../Game/NewPlayer.cpp ../Game/Grid.h
	  g++ -o Battleships Main.cpp ConnManager.cpp Client.cpp ../Game/NewPlayer.cpp

BattleshipsTest: Tests/Test.cpp ConnManager.h ConnManager.cpp Client.h Client.cpp ../Game/Battleship.h ../Game/NewPlayer.h ../Game/NewPlayer.cpp ../Game/Grid.h
	  g++ -o Battleships Tests/Test.cpp ConnManager.cpp Client.cpp ../Game/NewPlayer.cpp

a.out: Main.cpp ConnManager.h ConnManager.cpp Client.h Client.cpp ../Game/Battleship.h ../Game/NewPlayer.h ../Game/NewPlayer.cpp ../Game/Grid.h
	   g++ -g Main.cpp ConnManager.cpp Client.cpp ../Game/NewPlayer.cpp