#include "Map.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Loads each map file and prints whether it is valid
void testLoadMaps(const std::vector<std::string>& paths) {
    MapLoader loader;

    for (const std::string& path : paths) {
        // file name is everything after the last '/'
        std::string name = path.substr(path.find_last_of('/') + 1);

        Map* map = nullptr;

        try {
            map = loader.loadMap(path);
        }
        catch (const std::exception& e) {
            std::cout << name << ": invalid\n";
            continue;
        }

        if (map->validate()) {
            std::cout << name << ": valid\n";
        }
        else {
            std::cout << name << ": invalid\n";
        }

        delete map;
    }
}

#ifdef MAP_STANDALONE
int main(int argc, char* argv[]) {
    std::vector<std::string> paths(argv + 1, argv + argc);
    if (paths.empty()) {
        std::string assets = ASSETS_DIR;
        paths = {
            assets + "/artic/artic.map",
            assets + "/bigeurope/bigeurope.map",
            assets + "/europass/europass.map",
            assets + "/europe/europe.map",
            assets + "/Falkland-Islands/Falkland-Islands.map",
            assets + "/google/google.map",
            assets + "/kosova/kosova.map",
            assets + "/MiddleEast-Qatar/MiddleEast-Qatar.map",
            assets + "/solar/solar.map",
            assets + "/spain/spain.map",
            assets + "/sudamerica/sudamerica.map",
            assets + "/sw_baltic/sw_baltic.map",
            assets + "/us_map/us.map"
        };
    }
    testLoadMaps(paths);
    return 0;
}
#endif
