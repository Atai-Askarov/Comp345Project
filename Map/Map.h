#pragma once

#include <iostream>
#include <string>
#include <vector>

class Player;
class Territory;

class Continent {
private:
    std::string name;
    int bonus;
    std::string color;
    std::vector<Territory*> territories;

public:
    Continent(const std::string& name, int bonus, const std::string& color);
    Continent(const Continent& other);
    Continent& operator=(const Continent& other);

    void addTerritory(Territory* territory);

    std::string getName() const;
    int getBonus() const;
    std::string getColor() const;
    const std::vector<Territory*>& getTerritories() const;

    friend std::ostream& operator<<(std::ostream& os, const Continent& continent);
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
    Territory(const Territory& other);
    Territory& operator=(const Territory& other);

    void addNeighbour(Territory* neighbour);
    const std::vector<Territory*>& getNeighbours() const;

    int getId() const;
    std::string getName() const;
    Continent* getContinent() const;
    

    int getArmies() const;
    void setArmies(int armies);

    Player* getOwner() const;
    void setOwner(Player* owner);

    

    friend std::ostream& operator<<(std::ostream& os, const Territory& territory);
};

class Map {
private:
    std::vector<Territory*> countries;
    std::vector<Continent*> continents;
    void copyFrom(const Map& other);
    void clear();

public:
    Map();
    Map(const Map& other);
    Map& operator=(const Map& other);
    ~Map();

    void addCountry(Territory* country);
    void addContinent(Continent* continent);

    const std::vector<Territory*>& getCountries() const;
    const std::vector<Continent*>& getContinents() const;
    Territory* getTerritory(int id) const;
    Continent* getContinent(int index) const;

    bool isConnected() const;
    bool continentsAreConnected() const;
    bool eachTerritoryHasOneContinent() const;
    bool validate() const;

    friend std::ostream& operator<<(std::ostream& os, const Map& map);
};

class MapLoader {
public:
    MapLoader();
    MapLoader(const MapLoader& other);
    MapLoader& operator=(const MapLoader& other);
    Map* loadMap(const std::string& path);

    friend std::ostream& operator<<(std::ostream& os, const MapLoader& loader);
};
