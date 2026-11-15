#include "GameEngine.h"
#include "LoggingObserver.h"

#include <iostream>
#include <string>
#include <stdexcept>

/**
 * runs the GameEngine startup phase, allowing the user to enter commands
 * to load a map, validate it, add players, and start the game.
 */
void startupPhaseDriver()
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

    LogObserver logObserver;
    GameEngine engine;
    engine.attach(&logObserver);
    if (engine.getCommandProcessor() != nullptr)
    {
        engine.getCommandProcessor()->attach(&logObserver);
    }

    engine.startupPhase();

    std::cout << "\n--- Post-Startup Verification ---\n";
    std::cout << "Current Engine State: [" << engine.getCurrentStateName() << "]\n";

    // prints the players in turn order and their reinforcement pools, as well as the remaining deck size
    if (engine.getPlayers() != nullptr)
    {
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
    }

    if (engine.getDeck() != nullptr)
    {
        std::cout << "Remaining Deck Size: " << engine.getDeck()->getSize() << " cards\n";
    }

    engine.detach(&logObserver);
    if (engine.getCommandProcessor() != nullptr)
    {
        engine.getCommandProcessor()->detach(&logObserver);
    }
}

/**
 * Console driver for the GameEngine. The user types commands that trigger
 * state transitions as depicted in the assignment's state diagram.
 * Invalid commands are rejected with an error message and do not change the state.
 * The program ends when the "quit" command is entered in the "win" state.
 */

int main()
{
    // run the startup phase driver to demonstrate the GameEngine's functionality
    startupPhaseDriver();

    return 0;
}

/** Call separately once Parts 2/5 build. Existing startup main is unchanged.
 * Orders below are the real teammate classes, currently effect placeholders.
 * Ownership changes in case 6 are a fixture, not simulated combat.
 */
void part3Driver()
{
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
    std::cout << "1. Reinforcement: 2 territories + bonus 5 = 8; 11 territories = 3\n";
    engine.reinforcementPhase(map, players);
    check(a.getReinforcementPool() == 8 && b.getReinforcementPool() == 3);
    b.addTerritory(map.getCountries()[0]);
    engine.reinforcementPhase(map, players);
    check(a.getReinforcementPool() == 11 && b.getReinforcementPool() == 7);
    a.addTerritory(map.getCountries()[0]);

    std::cout << "2. Only Deploy while armies remain unallocated\n";
    int remaining = a.getReinforcementPool(), nextOrder = 0;
    check(a.issueOrder(&deck, remaining, nextOrder) && remaining == 0);
    check(ordersA.getOrder(0)->getName() == "Deploy" && a.getReinforcementPool() == 11);
    std::cout << "3. Friendly/enemy Advance selections; order payloads await Part 4\n";
    check(a.issueOrder(&deck, remaining, nextOrder) && ordersA.getOrder(1)->getName() == "Advance");
    check(a.issueOrder(&deck, remaining, nextOrder) && ordersA.getOrder(2)->getName() == "Advance");
    std::cout << "4. Existing Bomb card play queues one order and returns the card\n";
    hand.addCard(new Card(CardType::Bomb));
    check(a.issueOrder(&deck, remaining, nextOrder) && ordersA.getOrder(3)->getName() == "Bomb");
    check(hand.getSize() == 0 && deck.getSize() == 1 && !a.issueOrder(&deck, remaining, nextOrder));
    while (ordersA.getOrdersListSize())
        ordersA.remove(0); // End isolated issuing case.
    std::cout << "Round-robin issuing/execution; current order effects are placeholders\n";
    engine.issueOrdersPhase(players, &deck);
    ordersA.move(0, 1); // Deploy must execute first even behind an Advance.
    engine.executeOrdersPhase(map, players);
    check(ordersA.getOrdersListSize() == 0 && ordersB.getOrdersListSize() == 0);
    engine.executeOrdersPhase(map, players); // Empty queues.

    std::cout << "5. Zero-territory Carol is removed without deletion\n";
    engine.eliminatePlayers(map, players);
    check(players.size() == 2 && players[0] == &a && players[1] == &b && c.getName() == "Carol");
    std::cout << "6. Controlled ownership fixture, NOT a conquest test\n";
    check(!engine.checkVictory(map, players));
    for (size_t i = 0; i + 1 < map.getCountries().size(); ++i)
        a.addTerritory(map.getCountries()[i]);
    b.removeTerritory(map.getCountries().back());
    engine.eliminatePlayers(map, players);
    check(!engine.checkVictory(map, players)); // One active player is insufficient.
    a.addTerritory(map.getCountries().back());
    check(engine.checkVictory(map, players) == &a && engine.getCurrentStateName() == "win");
    std::cout << "Part 3 fixture checks passed; full gameplay awaits teammate integration.\n";
}
