#include "Player.h"
#include "../Cards/Cards.h"
#include "../Map/Map.h"
#include "../Orders/Orders.h"

#include <iostream>
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
        // Borrowed lists must outlive the Players, including their copies.
        OrdersList orders1;
        OrdersList orders2;
        Map map;
        auto continent = make_unique<Continent>("Demo Continent", 0, "blue");
        map.addContinent(continent.get());
        Continent* demoContinent = continent.release();

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
        cout << "Counts may range from 0 to 100.\n";
        int nextId = 1;
        for (Player* player : players) {
            int count = readNumber("How many territories for " + player->getName()
                                   + "? ", 0, 100);
            for (int i = 0; i < count; ++i) {
                string name = readName("Territory " + to_string(i + 1) + " name: ");
                auto territory = make_unique<Territory>(nextId++, name, demoContinent);
                map.addCountry(territory.get());
                Territory* added = territory.release();
                demoContinent->addTerritory(added);
                player->addTerritory(added);
            }
        }

        // A simple chain gives predictable neighbours without a map file.
        const vector<Territory*>& countries = map.getCountries();
        for (size_t i = 1; i < countries.size(); ++i) {
            countries[i - 1]->addNeighbour(countries[i]);
            countries[i]->addNeighbour(countries[i - 1]);
        }
        cout << "Territories form a two-way chain in the order entered:\n";
        printTerritories("Chain: ", countries);
        cout << "Each territory neighbours the entries immediately beside it.\n";
        cout << "If both Players have territories, their boundary territories can attack each other.\n";

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

        // Show each original list before copying/assignment shares pointers.
        cout << "\n=== 8. Orders ===\n";
        cout << "Adding a deploy order for " << player1.getName() << ".\n";
        player1.issueOrder("deploy");
        cout << player1.getName() << " orders: " << *player1.getOrdersList();
        cout << "Adding an advance order for " << player2.getName() << ".\n";
        player2.issueOrder("advance");
        cout << player2.getName() << " orders: " << *player2.getOrdersList();
        cout << "Orders are stored, not executed.\n\n";

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
