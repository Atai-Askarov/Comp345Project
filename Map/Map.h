#pragma once
#include <string>
#include <vector>

class Map;
class Territory;
class Player;

class MapLoader {
public:
    MapLoader();
    Map* loadMap(const std::string& path);
};

class Continent {
private:
    std::string name;
    int bonus;
    std::string color;
    std::vector<Territory*> territories;
public:
    Continent(const std::string& name, int bonus, const std::string& color);
    void addTerritory(Territory* territory);
};

class Territory {
private:
    int id;
    std::string name;
    Continent* continent;
    int armies;
    Player* player;
    std::vector<Territory*> neighbours;
public:
    Territory(int id, const std::string& name, Continent* continent);
    void addNeighbour(Territory*);
};

class Map {
private:
    std::vector<Territory*> countries;
    std::vector<Continent*> continents;
public:
    Map() {}
    void addCountry(Territory* country);
    void addContinent(Continent* continent);
    std::vector<Continent*> getContinents();
    std::vector<Territory*> getCountries();
    Territory* getTerritory(int index);
    Continent* getContinent(int index);
};