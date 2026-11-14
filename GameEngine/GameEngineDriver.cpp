#include "GameEngine.h"
#include "LoggingObserver.h"

#include <iostream>
#include <string>

/**
 * runs the GameEngine startup phase, allowing the user to enter commands
 * to load a map, validate it, add players, and start the game.
 */
void startupPhaseDriver() {
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
    if (engine.getCommandProcessor() != nullptr) {
        engine.getCommandProcessor()->attach(&logObserver);
    }

    engine.startupPhase();

    std::cout << "\n--- Post-Startup Verification ---\n";
    std::cout << "Current Engine State: [" << engine.getCurrentStateName() << "]\n";

    // prints the players in turn order and their reinforcement pools, as well as the remaining deck size
    if (engine.getPlayers() != nullptr) {
        std::cout << "Players in turn order (" << engine.getPlayers()->size() << "):\n";
        for (size_t i = 0; i < engine.getPlayers()->size(); ++i) {
            Player* p = (*engine.getPlayers())[i];
            std::cout << "  Turn " << (i + 1) << " -> " << *p
                      << " (Reinforcement Pool: " << p->getReinforcementPool() << ")\n";
            if (p->getHand() != nullptr) {
                std::cout << "    " << *(p->getHand());
            }
        }
    }

    if (engine.getDeck() != nullptr) {
        std::cout << "Remaining Deck Size: " << engine.getDeck()->getSize() << " cards\n";
    }

    engine.detach(&logObserver);
    if (engine.getCommandProcessor() != nullptr) {
        engine.getCommandProcessor()->detach(&logObserver);
    }
}

/**
* Console driver for the GameEngine. The user types commands that trigger
 * state transitions as depicted in the assignment's state diagram.
 * Invalid commands are rejected with an error message and do not change the state.
 * The program ends when the "quit" command is entered in the "win" state.
 */

int main() {
    // run the startup phase driver to demonstrate the GameEngine's functionality
    startupPhaseDriver();

    return 0;
}
