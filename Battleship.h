#pragma once

#include <iostream>

using namespace std;

class GeneralBattleship
{
public:
    char orientation;
    int length;
    int row, col;

    GeneralBattleship(char orientation, int length, int row, int col) : orientation(orientation), length(length), row(row), col(col) {}
    ~GeneralBattleship() {}

    virtual void printSymbol() = 0;
    int getLength()
    {
        return this->length;
    }
};

class AircraftCarrier : public GeneralBattleship
{
public:
    using GeneralBattleship::GeneralBattleship;
    AircraftCarrier(char orientation, int row, int col) : GeneralBattleship(orientation, 5, row, col) {}

    void printSymbol()
    {
        cout << "A\t";
    }
};

class Battleship : public GeneralBattleship
{
public:
    using GeneralBattleship::GeneralBattleship;
    Battleship(char orientation, int row, int col) : GeneralBattleship(orientation, 4, row, col) {}

    void printSymbol()
    {
        cout << "B\t";
    }
};

class Cruiser : public GeneralBattleship
{
public:
    using GeneralBattleship::GeneralBattleship;
    Cruiser(char orientation, int row, int col) : GeneralBattleship(orientation, 3, row, col) {}

    void printSymbol()
    {
        cout << "C\t";
    }
};

class Destroyer : public GeneralBattleship
{
public:
    using GeneralBattleship::GeneralBattleship;
    Destroyer(char orientation, int row, int col) : GeneralBattleship(orientation, 2, row, col) {}

    void printSymbol()
    {
        cout << "D\t";
    }
};

class Submarine : public GeneralBattleship
{
public:
    using GeneralBattleship::GeneralBattleship;
    Submarine(char orientation, int row, int col) : GeneralBattleship(orientation, 1, row, col) {}

    void printSymbol()
    {
        cout << "S\t";
    }
};