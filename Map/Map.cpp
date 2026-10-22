#include "Map.h"

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <queue>

enum class DocumentSection {
    Continents,
    Countries,
    Borders,
    Ignore  // any section we don't need, e.g. [files]
};

//Continent 
//************************

// Constructors
Continent::Continent(const std::string& name, int bonus, const std::string& color)
    : name(name), bonus(bonus), color(color) {
}

Continent::Continent(const Continent& other)
    : name(other.name), bonus(other.bonus), color(other.color), territories(other.territories) {
}

Continent& Continent::operator=(const Continent& other) {
    if (this != &other) {
        name = other.name;
        bonus = other.bonus;
        color = other.color;
        territories = other.territories;
    }
    return *this;
}

std::string Continent::getName() const {
    return name;
}

int Continent::getBonus() const {
    return bonus;
}

std::string Continent::getColor() const {
    return color;
}

const std::vector<Territory*>& Continent::getTerritories() const {
    return territories;
}

void Continent::addTerritory(Territory* territory) {
    territories.push_back(territory);
}

std::ostream& operator<<(std::ostream& os, const Continent& continent) {
    os << "Continent " << continent.name
       << " (bonus " << continent.bonus
       << ", color " << continent.color
       << ", " << continent.territories.size() << " territories)";
    return os;
}
//************************

//Territory
//************************
// Constructors
Territory::Territory(int id, const std::string& name, Continent* continent)
    : id(id), name(name), continent(continent), armies(0), player(nullptr) {
}

Territory::Territory(const Territory& other)
    : id(other.id), name(other.name), continent(other.continent),
      armies(other.armies), player(other.player), neighbours(other.neighbours) {
}

Territory& Territory::operator=(const Territory& other) {
    if (this != &other) {
        id = other.id;
        name = other.name;
        continent = other.continent;
        armies = other.armies;
        player = other.player;
        neighbours = other.neighbours;
    }
    return *this;
}

// Getters and Setters
const std::vector<Territory*>& Territory::getNeighbours() const {
    return neighbours;
}

int Territory::getId() const {
    return id;
}

std::string Territory::getName() const {
    return name;
}

Continent* Territory::getContinent() const {
    return continent;
}

int Territory::getArmies() const {
    return armies;
}

void Territory::setArmies(int newArmies) {
    armies = newArmies;
}

Player* Territory::getOwner() const {
    return player;
}

void Territory::setOwner(Player* owner) {
    player = owner;
}

void Territory::addNeighbour(Territory* neighbour) {
    neighbours.push_back(neighbour);
}

std::ostream& operator<<(std::ostream& os, const Territory& territory) {
    os << "Territory " << territory.id << " " << territory.name
       << " (continent " << (territory.continent ? territory.continent->getName() : "none")
       << ", armies " << territory.armies
       << ", " << (territory.player ? "owned" : "unowned")
       << ", neighbours:";
    for (Territory* neighbour : territory.neighbours) {
        os << " " << neighbour->getId();
    }
    os << ")";
    return os;
}
//************************

//Map stuff
//************************
Map::Map() {}

Map::Map(const Map& other) {
    copyFrom(other);
}

Map& Map::operator=(const Map& other) {
    if (this != &other) {
        clear();
        copyFrom(other);
    }
    return *this;
}

Map::~Map() {
    clear();
}

const std::vector<Territory*>& Map::getCountries() const {
    return countries;
}

const std::vector<Continent*>& Map::getContinents() const {
    return continents;
}

Territory* Map::getTerritory(int id) const {
    for (Territory* territory : countries) {
        if (territory->getId() == id) {
            return territory;
        }
    }
    return nullptr;
}

Continent* Map::getContinent(int index) const {
    if (index < 0 || index >= static_cast<int>(continents.size())) {
        return nullptr;
    }
    return continents[index];
}

// Deep copy: clones every continent and territory, then rewires the pointers
void Map::copyFrom(const Map& other) {
    // maps other's objects -> newly created objects
    std::map<const Continent*, Continent*> continentMap;
    std::map<const Territory*, Territory*> territoryMap;

    for (Continent* continent : other.continents) {
        Continent* copy = new Continent(continent->getName(), continent->getBonus(), continent->getColor());
        continentMap[continent] = copy;
        continents.push_back(copy);
    }

    for (Territory* territory : other.countries) {

        Continent* continent = territory->getContinent();
        Continent* copiedContinent = continent ? continentMap[continent] : nullptr;

        Territory* copy = new Territory(territory->getId(), territory->getName(), copiedContinent);
        copy->setArmies(territory->getArmies());
        copy->setOwner(territory->getOwner());

        territoryMap[territory] = copy;
        countries.push_back(copy);
    }

    for (Territory* territory : other.countries) {
        Territory* copy = territoryMap[territory];
        for (Territory* neighbour : territory->getNeighbours()) {
            copy->addNeighbour(territoryMap[neighbour]);
        }
    }

    for (Continent* continent : other.continents) {
        Continent* copy = continentMap[continent];
        for (Territory* territory : continent->getTerritories()) {
            copy->addTerritory(territoryMap[territory]);
        }
    }
}

void Map::clear() {
    for (Territory* territory : countries) {
        delete territory;
    }
    for (Continent* continent : continents) {
        delete continent;
    }
    countries.clear();
    continents.clear();
}

void Map::addCountry(Territory* country) {
    countries.push_back(country);
}

void Map::addContinent(Continent* continent) {
    continents.push_back(continent);
}

namespace {

void validateGraphConnectivityHelper(Territory* rootTerritory, std::unordered_set<Territory*>& seen, std::queue<Territory*>& yetToExplore, const std::unordered_set<Territory*>& allowed);

// Returns true if every territory in countries can be reached from the first one,
// moving only through territories that are in countries.
bool validateGraphConnectivity(const std::vector<Territory*>& countries) {
    if (countries.empty()) {
        return false;
    }

    std::unordered_set<Territory*> allowed(countries.begin(), countries.end());
    std::unordered_set<Territory*> seenTerritories;
    std::queue<Territory*> yetToExplore;

    Territory* rootTerritory = countries[0];
    seenTerritories.insert(rootTerritory);

    validateGraphConnectivityHelper(rootTerritory, seenTerritories, yetToExplore, allowed);

    return seenTerritories.size() == allowed.size();
}

// seen contains every territory discovered so far
// yetToExplore holds discovered territories whose neighbours haven't been checked yet
// allowed keeps the traversal inside the group being checked
void validateGraphConnectivityHelper(Territory* rootTerritory, std::unordered_set<Territory*>& seen, std::queue<Territory*>& yetToExplore, const std::unordered_set<Territory*>& allowed) {

    const std::vector<Territory*>& neighbours = rootTerritory->getNeighbours();

    for (Territory* territory : neighbours) {
        if (allowed.count(territory) == 0) {
            continue;  // outside the group
        }
        if (seen.count(territory) > 0) {
            continue;  // already discovered
        }
        seen.insert(territory);
        yetToExplore.push(territory);
    }

    if (yetToExplore.empty()) {
        return;
    }

    Territory* nextTerritory = yetToExplore.front();
    yetToExplore.pop();

    validateGraphConnectivityHelper(nextTerritory, seen, yetToExplore, allowed);
}

}

// The whole map is a connected graph
bool Map::isConnected() const {
    return validateGraphConnectivity(countries);
}

// Each continent is a connected subgraph
bool Map::continentsAreConnected() const {
    if (continents.empty()) {
        return false;
    }
    for (Continent* continent : continents) {
        if (!validateGraphConnectivity(continent->getTerritories())) {
            return false;
        }
    }
    return true;
}

// Each territory belongs to one and only one continent
bool Map::eachTerritoryHasOneContinent() const {
    std::map<Territory*, int> continentCount;
    for (Continent* continent : continents) {
        for (Territory* territory : continent->getTerritories()) {
            continentCount[territory]++;
        }
    }

    for (Territory* territory : countries) {
        if (territory->getContinent() == nullptr) {
            return false;
        }
        if (continentCount[territory] != 1) {
            return false;
        }
    }
    return true;
}

bool Map::validate() const {
    return isConnected() && continentsAreConnected() && eachTerritoryHasOneContinent();
}

std::ostream& operator<<(std::ostream& os, const Map& map) {
    os << "Map with " << map.continents.size() << " continents and "
       << map.countries.size() << " territories\n";
    for (Continent* continent : map.continents) {
        os << "  " << *continent << "\n";
    }
    for (Territory* territory : map.countries) {
        os << "  " << *territory << "\n";
    }
    return os;
}
//************************

//MapLoader stuff
//************************
MapLoader::MapLoader() {}

// MapLoader has no state, so copying has nothing to do
MapLoader::MapLoader(const MapLoader& other) {
    (void)other;
}

MapLoader& MapLoader::operator=(const MapLoader& other) {
    (void)other;
    return *this;
}

Map* MapLoader::loadMap(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    Map* map = new Map();
    DocumentSection documentSection = DocumentSection::Ignore;
    std::string word;
    std::string restOfLine;

    while (file >> word) {
        // comment: skip the rest of the line
        if (word[0] == ';') {
            std::getline(file, restOfLine);
            continue;
        }

        // section headers are case-insensitive
        std::string header = word;
        for (char& c : header) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        if (header == "[continents]") {
            documentSection = DocumentSection::Continents;
            continue;
        }
        else if (header == "[countries]") {
            documentSection = DocumentSection::Countries;
            continue;
        }
        else if (header == "[borders]") {
            documentSection = DocumentSection::Borders;
            continue;
        }
        else if (header == "[files]") {
            documentSection = DocumentSection::Ignore;
            continue;
        }

        switch (documentSection) {
        case DocumentSection::Continents: {
            // line format: name bonus color
            std::string name = word;
            std::string color;
            int bonus;
            file >> bonus >> color;
            map->addContinent(new Continent(name, bonus, color));
            break;
        }
        case DocumentSection::Countries:
        {
            // line format: id name continentId x y
            int countryId = std::stoi(word);
            std::string name;
            int continentId;
            file >> name >> continentId;
            std::getline(file, restOfLine);  // skip the x y coordinates

            // territories refer to continents based on enumeration starting from 1
            Continent* targetContinent = map->getContinent(continentId - 1);
            if (targetContinent == nullptr) {
                delete map;
                throw std::runtime_error("Unknown continent " + std::to_string(continentId) + " in file: " + path);
            }
            Territory* country = new Territory(countryId, name, targetContinent);
            targetContinent->addTerritory(country);
            map->addCountry(country);
            break;
        }
        case DocumentSection::Borders:
        {
            // line format: territoryId neighbourId neighbourId ...
            int territoryId = std::stoi(word);
            Territory* territory = map->getTerritory(territoryId);
            if (territory == nullptr) {
                delete map;
                throw std::runtime_error("Unknown territory " + std::to_string(territoryId) + " in file: " + path);
            }

            // read neighbour ids until the end of the line
            while (file.peek() != '\n' && file.peek() != EOF) {
                if (file.peek() == ' ' || file.peek() == '\r') {
                    file.get();
                    continue;
                }
                int neighbourId;
                file >> neighbourId;
                Territory* neighbouringTerritory = map->getTerritory(neighbourId);
                if (neighbouringTerritory == nullptr) {
                    delete map;
                    throw std::runtime_error("Unknown territory " + std::to_string(neighbourId) + " in file: " + path);
                }
                territory->addNeighbour(neighbouringTerritory);
            }
            break;
        }
        }
    }

    // the loop stops at the end of the file, or early if a value had the wrong type
    if (!file.eof()) {
        delete map;
        throw std::runtime_error("Bad format in file: " + path);
    }

    return map;
}

std::ostream& operator<<(std::ostream& os, const MapLoader& loader) {
    (void)loader;
    os << "MapLoader";
    return os;
}
//************************
