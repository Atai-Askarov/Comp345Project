#include "Player.h"
#include "../Cards/Cards.h"
#include "../Map/Map.h"
#include "../Orders/Orders.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

// Read a whole line so names can contain spaces. EOF ends the demonstration.
string readName(const string& prompt) {
    string text;
    while (true) {
        cout << prompt;
        if (!getline(cin, text)) {
            throw runtime_error("Input ended. Demonstration closed.");
        }
        size_t first = text.find_first_not_of(" \t\r");
        if (first != string::npos) {
            return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
        }
        cout << "Please enter a name or value.\n";
    }
}

// Read one integer per line, rejecting negative counts and extra text.
int readNumber(const string& prompt, int minimum, int maximum) {
    while (true) {
        string text = readName(prompt);
        istringstream input(text);
        int number;
        string extra;
        if ((input >> number) && !(input >> extra)
                && number >= minimum && number <= maximum) {
            return number;
        }
        cout << "Please enter a whole number from " << minimum
             << " to " << maximum << ".\n";
    }
}

void printTerritories(const string& label, const vector<Territory*>& territories) {
    cout << label;
    if (territories.empty()) {
        cout << "None";
    }
    for (size_t i = 0; i < territories.size(); ++i) {
        if (i > 0) {
            cout << ", ";
        }
        cout << territories[i]->getName();
    }
    cout << '\n';
}

void printPlayer(const Player& player) {
    cout << player << '\n';
    printTerritories("Owned territories: ", player.toDefend());
    cout << *player.getHand() << '\n';
}

int main() {
    try {
        // Hands and Map outlive all Players. Map deletes its territories;
        // each Hand deletes its cards. Players only borrow these objects.
        Hand hand1;
        Hand hand2;
        // Lists outlive all Players that borrow them.
        OrdersList orders1;
        OrdersList orders2;
        unique_ptr<Map> map;

        cout << "PlayerDriver\n\n=== 1. Create Players ===\n";
        string name1 = readName("Enter Player 1 name: ");
        string name2 = readName("Enter Player 2 name: ");
        Player player1(name1);
        Player player2(name2);
        player1.setHand(&hand1);
        player2.setHand(&hand2);
        player1.setOrdersList(&orders1);
        player2.setOrdersList(&orders2);
        Player* players[] = {&player1, &player2};

        cout << "\n=== 2. Assign Territories ===\n";
        const string mapFiles[] = {
            "artic/artic.map", "bigeurope/bigeurope.map", "europass/europass.map",
            "europe/europe.map", "Falkland-Islands/Falkland-Islands.map",
            "google/google.map", "kosova/kosova.map",
            "MiddleEast-Qatar/MiddleEast-Qatar.map", "solar/solar.map",
            "spain/spain.map", "sudamerica/sudamerica.map",
            "sw_baltic/sw_baltic.map", "us_map/us.map"
        };
        cout << "Choose a map (territories and borders come from its file):\n";
        for (int i = 0; i < 13; ++i) {
            cout << i + 1 << ". " << mapFiles[i] << '\n';
        }
        int choice = readNumber("Map number: ", 1, 13);
        string mapPath;
        // Support running from either the project root or the Player folder.
        string sourceFile = __FILE__;
        string sourceFolder = sourceFile.substr(0, sourceFile.find_last_of("/\\") + 1);
        const string folders[] = {"Map/Assets/", "../Map/Assets/",
                                  sourceFolder + "../Map/Assets/"};
        for (const string& folder : folders) {
            ifstream file(folder + mapFiles[choice - 1]);
            if (file) {
                mapPath = folder + mapFiles[choice - 1];
                break;
            }
        }
        if (mapPath.empty()) {
            throw runtime_error("Map file not found. Run from the project root or Player folder.");
        }
        MapLoader loader;
        map.reset(loader.loadMap(mapPath));
        if (!map->validate()) {
            throw runtime_error("The selected map is invalid.");
        }
        const vector<Territory*>& countries = map->getCountries();
        cout << "Available territories:\n";
        for (size_t i = 0; i < countries.size(); ++i) {
            cout << i + 1 << ". " << countries[i]->getName() << '\n';
        }
        int remaining = static_cast<int>(countries.size());
        for (Player* player : players) {
            int count = readNumber("How many territories for " + player->getName()
                                   + "? ", 0, remaining);
            for (int i = 0; i < count; ++i) {
                Territory* territory = nullptr;
                while (true) {
                    int number = readNumber("Choose territory " + to_string(i + 1)
                                            + " by number: ", 1,
                                            static_cast<int>(countries.size()));
                    territory = countries[number - 1];
                    if (!territory->getOwner()) {
                        break;
                    }
                    cout << "That territory is already assigned. Choose another.\n";
                }
                player->addTerritory(territory);
                --remaining;
            }
        }
        cout << "Borders are taken from the map; no extra connections are added.\n";
        cout << "Unassigned territories remain unowned and can also be attack targets.\n";

        cout << "\n=== 3. Assign Cards ===\n";
        CardType types[] = {CardType::Bomb, CardType::Reinforcement,
                            CardType::Blockade, CardType::Airlift, CardType::Diplomacy};
        for (Player* player : players) {
            int count = readNumber("How many cards for " + player->getName()
                                   + "? ", 0, 100);
            cout << "Choose by number: 1 Bomb, 2 Reinforcement, 3 Blockade,\n"
                 << "4 Airlift, 5 Diplomacy.\n";
            for (int i = 0; i < count; ++i) {
                int choice = readNumber("Choose card " + to_string(i + 1) + ": ", 1, 5);
                auto card = make_unique<Card>(types[choice - 1]);
                player->getHand()->addCard(card.get());
                card.release(); // The Hand now owns this card.
            }
        }

        cout << "\n=== 4. Player Summary ===\n";
        printPlayer(player1);
        printPlayer(player2);

        cout << "=== 5. Defend and Attack ===\n";
        for (Player* player : players) {
            cout << player->getName() << ":\n";
            printTerritories("To defend: ", player->toDefend());
            printTerritories("To attack: ", player->toAttack());
        }

        cout << "=== 6. Copy Constructor ===\n";
        cout << "Creating Player 3 as a copy of " << player1.getName() << "...\n";
        Player player3(player1);
        cout << "Player 3: " << player3.getName() << '\n';
        cout << "Cards: ";
        if (player3.getHand()->getSize() == 0) {
            cout << "None";
        }
        for (Card* card : *player3.getHand()->getCards()) {
            cout << *card << ' ';
        }
        cout << "\n\n";

        cout << "=== 7. Assignment Operator ===\n";
        cout << "Player 2 before: " << player2.getName() << '\n';
        // Assignment replaces this list, even for territories Player 2 owns.
        // Save it so we can restore its original ownership records afterward.
        vector<Territory*> originalTerritories = player2.toDefend();
        player2 = player1;
        // These territories still name player2 as owner. Re-registering them
        // repairs its list without transferring any of player1's territories.
        for (Territory* territory : originalTerritories) {
            player2.addTerritory(territory);
        }
        cout << "Player 2 after: " << player2.getName() << '\n';
        cout << "Cards: ";
        if (player2.getHand()->getSize() == 0) {
            cout << "None";
        }
        for (Card* card : *player2.getHand()->getCards()) {
            cout << *card << ' ';
        }
        cout << "\n\n";

        cout << "=== 8. Orders ===\n";
        // Assignment shared Player 1's list. Reattach Player 2's own empty
        // list so this section demonstrates each Player issuing orders.
        player2.setOrdersList(&orders2);
        cout << "Player 1 issues a Deploy order.\n";
        player1.issueOrder("deploy");
        cout << "Player 1 orders: " << *player1.getOrdersList();
        cout << "Player 2 issues an Advance order.\n";
        player2.issueOrder("advance");
        cout << "Player 2 orders: " << *player2.getOrdersList();
        cout << "Orders are stored, not executed.\n";

        // Clear owner back-pointers before the Players leave scope.
        for (Player* player : players) {
            for (Territory* territory : player->toDefend()) {
                player->removeTerritory(territory);
            }
        }
    }
    catch (const runtime_error& error) {
        cout << '\n' << error.what() << '\n';
    }
    return 0;
}
