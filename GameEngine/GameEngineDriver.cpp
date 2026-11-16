#include "GameEngine.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <algorithm>

/**
 * runs the GameEngine startup phase, allowing the user to enter commands
 * to load a map, validate it, add players, and start the game.
 */
void startupPhaseDriver(int argc, char *argv[])
{
    std::cout << "=========================================\n"
              << "   Part 2: Game Startup Phase Driver\n"
              << "=========================================\n"
              << "Example commands to enter:\n"
              << "  loadmap <mapfile.map>\n"
              << "  validatemap\n"
              << "  addplayer Alice\n"
              << "  addplayer Bob\n"
              << "  gamestart\n\n";

    GameEngine engine;
    FileCommandProcessorAdapter *fileProcessor = nullptr;
    if (argc >= 3 && std::string(argv[1]) == "-file")
    {
        fileProcessor = new FileCommandProcessorAdapter(&engine, argv[2]);
        engine.setCommandProcessor(fileProcessor);
    }
    engine.startupPhase();

    std::cout << "\n--- Post-Startup Verification ---\n";
    std::cout << "Current Engine State: [" << engine.getCurrentStateName() << "]\n";
    if (engine.getMap() != nullptr)
    {
        std::cout << "Map territories: " << engine.getMap()->getCountries().size() << "\n";
    }

    // prints the players in turn order and their reinforcement pools, as well as the remaining deck size
    if (engine.getPlayers() != nullptr)
    {
        size_t fewest = engine.getMap() ? engine.getMap()->getCountries().size() : 0, most = 0;
        for (Player *p : *engine.getPlayers())
        {
            fewest = std::min(fewest, p->toDefend().size());
            most = std::max(most, p->toDefend().size());
        }
        std::cout << "Players in turn order (" << engine.getPlayers()->size() << "):\n";
        for (size_t i = 0; i < engine.getPlayers()->size(); ++i)
        {
            Player *p = (*engine.getPlayers())[i];
            std::cout << "  Turn " << (i + 1) << " -> " << *p
                      << " (Reinforcement Pool: " << p->getReinforcementPool() << ")\n";
            if (p->getHand() != nullptr)
            {
                std::cout << "    " << *(p->getHand());
            }
        }
        if (!engine.getPlayers()->empty() && engine.getCurrentStateName() == "assign reinforcement")
        {
            std::cout << "Fair distribution (counts differ by <= 1): " << (most - fewest <= 1 ? "yes" : "NO") << "\n";
        }
    }

    if (engine.getDeck() != nullptr)
    {
        std::cout << "Remaining Deck Size: " << engine.getDeck()->getSize() << " cards\n";
    }
    engine.setCommandProcessor(nullptr);
    delete fileProcessor;
}

/**
 * Console driver for the GameEngine. The user types commands that trigger
 * state transitions as depicted in the assignment's state diagram.
 * Invalid commands are rejected with an error message and do not change the state.
 * The program ends when the "quit" command is entered in the "win" state.
 */

int main(int argc, char *argv[])
{
#ifdef PART3_MENU
    try { part3Driver(); }
    catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
#else
    // run the startup phase driver to demonstrate the GameEngine's functionality
    startupPhaseDriver(argc, argv);
#endif

    return 0;
}

namespace {
// Every call builds its own map, players, pools, hand and order lists.
// No new test classes are needed, and menu order cannot affect another case.
void runPart3Demonstration(int choice)
{
    Orders::endTurn(); // Prevent turn-state leakage between independent cases.
    auto check = [](bool condition)
    {
        if (!condition)
            throw std::logic_error("Part 3 demonstration failed.");
    };
    Map map;
    Deck deck;
    Hand hand;
    OrdersList ordersA, ordersB, ordersC;
    Player a("Alice"), b("Bob"), c("Carol");
    a.setHand(&hand);
    a.setOrdersList(&ordersA);
    b.setOrdersList(&ordersB);
    c.setOrdersList(&ordersC);
    auto *small = new Continent("Small", 5, "red");
    auto *large = new Continent("Large", 0, "blue");
    map.addContinent(small);
    map.addContinent(large);
    for (int i = 0; i < 13; ++i)
    {
        Continent *group = i < 2 ? small : large;
        auto *t = new Territory(i + 1, "T" + std::to_string(i + 1), group);
        map.addCountry(t);
        group->addTerritory(t);
        (i < 2 ? a : b).addTerritory(t);
        t->setArmies(2);
        if (i > 0)
        {
            Territory *previous = map.getCountries()[i - 1];
            previous->addNeighbour(t);
            t->addNeighbour(previous);
        }
    }
    std::vector<Player *> players{&a, &b, &c};
    GameEngine engine;
    Territory* first = map.getCountries()[0];
    Territory* second = map.getCountries()[1];
    Territory* enemy = map.getCountries()[2];

    switch (choice) {
    case 1: {
        std::cout << "\nReinforcement: Alice owns 2 territories and bonus 5; Bob owns 11.\n"
                  << "Before: Alice pool=" << a.getReinforcementPool() << ", Bob pool=" << b.getReinforcementPool() << '\n';
        engine.reinforcementPhase(map, players);
        check(a.getReinforcementPool() == 8 && b.getReinforcementPool() == 3);
        std::cout << "After: Alice pool=8, Bob pool=3\n";
        b.addTerritory(first);
        std::cout << "Next reinforcement: Alice owns 1 territory (no bonus); Bob owns 12.\n";
        engine.reinforcementPhase(map, players);
        check(a.getReinforcementPool() == 11 && b.getReinforcementPool() == 7);
        std::cout << "After: Alice pool=11 (+3), Bob pool=7 (+4). Existing armies were preserved.\n";
        break;
    }
    case 2: {
        a.addReinforcements(5); b.addReinforcements(3);
        std::cout << "\nBefore issuing: Alice pool=5, Bob pool=3; both order lists empty.\n"
                  << "Watch Alice and Bob alternate. Their first orders must be Deploy.\n";
        engine.issueOrdersPhase(players, &deck);
        check(ordersA.getOrder(0)->getName() == "Deploy" && ordersB.getOrder(0)->getName() == "Deploy");
        check(a.getReinforcementPool() == 5 && b.getReinforcementPool() == 3);
        check(first->getArmies() == 2 && enemy->getArmies() == 2);
        std::cout << "After issuing: Alice " << ordersA << "Bob " << ordersB
                  << "All pool armies are allocated; execution, not issuing, spends them.\n";
        ordersA.move(0, 1); // Still execute Deploy first, even behind Advance.
        std::cout << "Executing: expect Alice Deploy, Bob Deploy, then alternating Advances.\n";
        engine.executeOrdersPhase(map, players);
        check(a.getReinforcementPool() == 0 && b.getReinforcementPool() == 0);
        check(ordersA.getOrdersListSize() == 0 && ordersB.getOrdersListSize() == 0);
        engine.executeOrdersPhase(map, players); // Empty queues are harmless.
        std::cout << "After execution: both pools=0 and both lists empty.\n";
        break;
    }
    case 3: {
        int remaining = 0, nextOrder = 0;
        std::cout << "\nBefore: T1=2, T2=2 (Alice); T3=2 (Bob).\n";
        check(a.issueOrder(&deck, remaining, nextOrder) && ordersA.getOrder(0)->getName() == "Advance");
        check(a.issueOrder(&deck, remaining, nextOrder) && ordersA.getOrder(1)->getName() == "Advance");
        ordersA.getOrder(0)->execute(); ordersA.remove(0);
        check(first->getArmies() == 1 && second->getArmies() == 3);
        std::cout << "After friendly T1 -> T2: T1=1, T2=3.\n"
                  << "Now execute the real T2 -> T3 attack; combat results vary.\n";
        ordersA.getOrder(0)->execute(); ordersA.remove(0);
        check(second->getArmies() >= 1 && second->getArmies() <= 3
              && enemy->getArmies() >= 0 && enemy->getArmies() <= 2
              && (enemy->getOwner() == &a || enemy->getOwner() == &b));
        check(!a.issueOrder(&deck, remaining, nextOrder));
        std::cout << "After attack: T2=" << second->getArmies() << ", T3=" << enemy->getArmies()
                  << ". The attack uses both available source armies; ownership may change.\n";
        break;
    }
    case 4: {
        int remaining = 0, nextOrder = 2; // Isolate card issuing from Advance decisions.
        hand.addCard(new Card(CardType::Bomb));
        std::cout << "\nBefore card play: hand=1, deck=0, orders=0, enemy T3 armies=2.\n";
        check(a.issueOrder(&deck, remaining, nextOrder));
        check(hand.getSize() == 0 && deck.getSize() == 1 && ordersA.getOrder(0)->getName() == "Bomb");
        check(enemy->getArmies() == 2); // No execution during issuing.
        std::cout << "After issuing: hand=0, deck=1, orders=1; T3 still has 2 armies.\n";
        ordersA.getOrder(0)->execute(); ordersA.remove(0);
        check(enemy->getArmies() == 1 && !a.issueOrder(&deck, remaining, nextOrder));
        std::cout << "After execution: enemy T3 armies=1.\n";
        break;
    }
    case 5:
        std::cout << "\nBefore: 3 active players. Alice owns 2, Bob 11, Carol 0 territories.\n";
        engine.eliminatePlayers(map, players);
        check(players.size() == 2 && players[0] == &a && players[1] == &b && c.getName() == "Carol");
        std::cout << "After: 2 active players, Alice then Bob. Carol is removed, not deleted.\n";
        break;
    case 6:
        std::cout << "\nControlled ownership fixture, NOT a conquest test.\n"
                  << "Before: Alice owns 2/13 territories. No winner.\n";
        check(!engine.checkVictory(map, players));
        for (size_t i = 0; i + 1 < map.getCountries().size(); ++i) a.addTerritory(map.getCountries()[i]);
        b.removeTerritory(map.getCountries().back());
        engine.eliminatePlayers(map, players);
        check(!engine.checkVictory(map, players));
        std::cout << "Alice now owns 12/13; the last is unowned. Still no winner despite one active player.\n";
        a.addTerritory(map.getCountries().back());
        check(engine.checkVictory(map, players) == &a && engine.getCurrentStateName() == "win");
        std::cout << "After: Alice owns all 13 territories; engine state=win.\n";
        break;
    }
    Orders::endTurn();
    std::cout << "PASS: demonstration " << choice << '\n';
}

// Watches the real phase transitions; never issues orders or changes ownership.
struct SimulationTurnLimit {};

class FullGameObserver : public Observer {
public:
    FullGameObserver(GameEngine* game, Player* alice, Player* bob)
        : engine(game), players{alice, bob} { engine->attach(this); }
    ~FullGameObserver() override { engine->detach(this); }

    void printState() const {
        for (Player* player : players) {
            std::cout << player->getName() << " owns";
            const auto owned = player->toDefend();
            if (owned.empty()) std::cout << " no territories";
            for (Territory* territory : owned)
                std::cout << ' ' << territory->getName() << " (" << territory->getArmies() << " armies)";
            std::cout << '\n';
        }
    }

    void finishTurn() {
        if (!executing) return;
        std::cout << "\nEND OF TURN\n";
        printState();
        for (int i = 0; i < 2; ++i) {
            if (participating[i] && players[i]->toDefend().empty()) {
                std::cout << players[i]->getName() << " was eliminated.\n";
                participating[i] = false;
            }
        }
        executing = false;
    }

    void update(ILoggable*) override {
        const std::string state = engine->getCurrentStateName();
        if (state == "assign reinforcement") {
            if (executing) { finishTurn(); return; }
            if (turns == 100) throw SimulationTurnLimit{};
            std::cout << "\n========== TURN " << ++turns << " ==========\n\nINITIAL STATE\n";
            printState();
            for (int i = 0; i < 2; ++i) poolsBefore[i] = players[i]->getReinforcementPool();
            std::cout << "\nREINFORCEMENT\n";
        } else if (state == "issue orders") {
            for (int i = 0; i < 2; ++i)
                if (participating[i])
                    std::cout << players[i]->getName() << " receives "
                              << players[i]->getReinforcementPool() - poolsBefore[i] << " armies\n";
            std::cout << "\nISSUE ORDERS\n";
        } else if (state == "execute orders") {
            executing = true;
            std::cout << "\nEXECUTE ORDERS\n";
        }
    }

    int getTurns() const { return turns; }

private:
    GameEngine* engine;
    Player* players[2];
    int poolsBefore[2] = {0, 0};
    bool participating[2] = {true, true};
    int turns = 0;
    bool executing = false;
};

void fullGameSimulation()
{
    Map map;
    Deck deck;
    Hand handA, handB;
    OrdersList ordersA, ordersB;
    Player alice("Alice"), bob("Bob");
    alice.setHand(&handA); bob.setHand(&handB);
    alice.setOrdersList(&ordersA); bob.setOrdersList(&ordersB);
    auto* continent = new Continent("Arena", 0, "red");
    map.addContinent(continent);
    auto* first = new Territory(1, "T1", continent);
    auto* second = new Territory(2, "T2", continent);
    for (Territory* territory : {first, second}) {
        map.addCountry(territory);
        continent->addTerritory(territory);
        territory->setArmies(2);
    }
    first->addNeighbour(second); second->addNeighbour(first);
    alice.addTerritory(first); bob.addTerritory(second);
    if (!map.validate()) throw std::logic_error("Simulation map is invalid.");
    for (int i = 0; i < 5; ++i) deck.addCard(new Card(CardType::Bomb));
    std::vector<Player*> players{&alice, &bob};
    GameEngine engine;
    FullGameObserver observer(&engine, &alice, &bob);
    bool limited = false;
    Orders::endTurn();
    try { engine.mainGameLoop(map, players, &deck); }
    catch (const SimulationTurnLimit&) { limited = true; }
    observer.finishTurn();
    std::cout << "\n========== GAME OVER ==========\n";
    if (limited) std::cout << "No winner reached within 100 turns. Demonstration stopped safely.\n";
    else {
        Player* winner = engine.checkVictory(map, players);
        if (!winner) throw std::logic_error("Simulation ended without a winner.");
        std::cout << "Winner: " << winner->getName() << '\n';
    }
    std::cout << "Turns played: " << observer.getTurns()
              << "\nGame state: " << engine.getCurrentStateName() << '\n';
    // A winning order can leave the opponent's orders pending. The lists own them.
    while (ordersA.getOrdersListSize()) ordersA.remove(0);
    while (ordersB.getOrdersListSize()) ordersB.remove(0);
    Orders::resetGameState(); // Map and players are still alive here.
}
} // namespace

// The default main still runs startup; build with -DPART3_MENU for this menu.
void part3Driver()
{
    while (true) {
        std::cout << "\n=== Part 3 Demonstrations ===\n"
                  << "1. Reinforcement Phase\n2. Issue Orders Phase\n"
                  << "3. Execute Orders Phase\n4. Card Demonstration\n"
                  << "5. Player Elimination\n6. Victory Detection\n"
                  << "7. Run All Demonstrations\n8. Full Game Simulation\n9. Exit\nChoice: " << std::flush;
        std::string line, extra;
        if (!std::getline(std::cin, line)) return;
        std::istringstream input(line);
        int choice;
        if (!(input >> choice) || (input >> extra) || choice < 1 || choice > 9) {
            std::cout << "Please enter a whole number from 1 to 9.\n";
            continue;
        }
        if (choice == 9) return;
        if (choice == 8) { fullGameSimulation(); continue; }
        if (choice == 7) {
            for (int demonstration = 1; demonstration <= 6; ++demonstration)
                runPart3Demonstration(demonstration);
            std::cout << "All six demonstrations passed using fresh fixtures.\n";
        } else {
            runPart3Demonstration(choice);
        }
    }
}
