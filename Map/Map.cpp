#include "Map.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class DocumentSection {
    Continents,
    Countries,
    Borders,
    Files,
    Skippable
};

//Continent stuff
//************************
Continent::Continent(const std::string& name, int bonus, const std::string& color)
    : name(name), bonus(bonus), color(color) {
}

void Continent::addTerritory(Territory* territory) {
    territories.push_back(territory);
}
//************************

//Territory Stuff 
//************************
Territory::Territory(int id, const std::string& name, Continent* continent)
    : id(id), name(name), continent(continent), armies(0), player(nullptr) {
}
void Territory::addNeighbour(Territory* territory) {
    neighbours.push_back(territory);


}

//************************

//Map stuff
//************************
void Map::addCountry(Territory* territory) {
    countries.push_back(territory);
}

void Map::addContinent(Continent* continent) {
    continents.push_back(continent);
}

std::vector<Territory*> Map::getCountries() {
    return countries;
}

std::vector<Continent*> Map::getContinents() {
    return continents;
}

Territory* Map::getTerritory(int index) {
    return countries[index -1];
}

Continent* Map::getContinent(int index) {
    return continents[index];
}
//************************

//MapLoader stuff
//************************
MapLoader::MapLoader() {}

Map* MapLoader::loadMap(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }

    Map* map = new Map();
    DocumentSection documentSection = DocumentSection::Skippable;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty() || line[0] == ';')
            continue;

        if (line == "[Continents]") {
            documentSection = DocumentSection::Continents;
            continue;
        }
        else if (line == "[Countries]") {
            documentSection = DocumentSection::Countries;
            continue;
        }
        else if (line == "[Borders]") {
            documentSection = DocumentSection::Borders;
            continue;
        }
        else if (line == "[Files]") {
            documentSection = DocumentSection::Files;
            continue;
        }

 

        std::istringstream stringStream(line);
        switch (documentSection) {
        case DocumentSection::Continents: {
            std::string name, color;
            int bonus;
            if (stringStream >> name >> bonus >> color) {
                map->addContinent(new Continent(name, bonus, color));
            }
            break;
        }
        case DocumentSection::Countries:
        {
            std::string name;
            int countryId, continentId;
            if (stringStream >> countryId >> name >> continentId) {
                // territories refer to continents based on enumeration starting from 1
                Continent* targetContinent = map->getContinent(continentId - 1);
                Territory* country = new Territory(countryId, name, targetContinent);
                targetContinent->addTerritory(country);
                map->addCountry(country);
            }
            break;
        }
        case DocumentSection::Borders:
        {
            int territoryId;
            int neighbourId;

            if (!(stringStream >> territoryId))
                break;

            Territory* territory = map->getTerritory(territoryId);

            while (stringStream >> neighbourId) {
                Territory* neighbouringTerritory = map->getTerritory(neighbourId);
                territory->addNeighbour(neighbouringTerritory);
            }
            break;
        }
        case DocumentSection::Files:
            break;
        case DocumentSection::Skippable:
            break;
        }
    }

    return map;
}
//************************