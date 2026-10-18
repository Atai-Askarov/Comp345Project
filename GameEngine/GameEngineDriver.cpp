#include "GameEngine.h"

#include <iostream>
#include <string>

/**
 * Console driver for the GameEngine. The user types commands that trigger
 * state transitions as depicted in the assignment's state diagram.
 * Invalid commands are rejected with an error message and do not change the state.
 * The program ends when the "end" command is entered in the "win" state.
 */
int main() {
    GameEngine engine;

    std::cout << "=== Warzone Game Engine ===\n"
              << "Type a command to move through the game states.\n\n";

    std::string input;
    while (!engine.isFinished()) {
        std::cout << "Current state: " << engine.getCurrentStateName() << "\n";
        std::cout << "Valid commands:";
        for (const std::string& command : engine.getValidCommands()) {
            std::cout << " " << command;
        }
        std::cout << "\n> ";

        if (!std::getline(std::cin, input)) {
            std::cout << "\nInput closed. Exiting." << std::endl;
            return 0;
        }
        engine.processCommand(input);
        std::cout << std::endl;
    }

    std::cout << "Game over. Thanks for playing!" << std::endl;
    return 0;
}
