#include "CommandProcessing.h"

#include "../GameEngine/GameEngine.h"

#include <iostream>
#include <string>

namespace {

/**
 * Plays the part of the game itself during the play phase. The user cannot type
 * these commands, so the driver triggers them (issueorder, endissueorders and win)
 * until the game reaches the win state, where the user may type replay or quit.
 */
void triggerGameEvents(GameEngine& engine) {
    while (true) {
        std::string state = engine.getCurrentStateName();
        std::string event;
        if (state == "assign reinforcement") {
            event = "issueorder";
        } else if (state == "issue orders") {
            event = "endissueorders";
        } else if (state == "execute orders") {
            event = "win";
        } else {
            return;
        }
        std::cout << "[game event] ";
        engine.processCommand(event);
    }
}

/** Prints how to start the program. */
void printUsage(const std::string& program) {
    std::cout << "Usage:\n"
              << "  " << program << " -console\n"
              << "  " << program << " -file <filename>\n";
}

} // namespace

/**
 * Driver for Part 1. The command line chooses where commands come from:
 * -console reads them from the keyboard (CommandProcessor), -file <filename>
 * reads them from a text file (FileCommandProcessorAdapter). Every command is
 * validated against the game state: valid ones change the state, invalid ones
 * are rejected with an error saved in the command's effect.
 */
int main(int argc, char* argv[]) {
    std::string program = argc > 0 ? argv[0] : "commandprocessing";
    bool useConsole = false;
    std::string fileName;

    if (argc == 2 && std::string(argv[1]) == "-console") {
        useConsole = true;
    } else if (argc == 3 && std::string(argv[1]) == "-file") {
        fileName = argv[2];
    } else {
        printUsage(program);
        return 1;
    }

    GameEngine engine;
    CommandProcessor* processor = nullptr;

    if (useConsole) {
        processor = new CommandProcessor(&engine);
        std::cout << "Reading commands from the console.\n";
    } else {
        FileCommandProcessorAdapter* fileProcessor = new FileCommandProcessorAdapter(&engine, fileName);
        if (!fileProcessor->isReady()) {
            std::cout << "Error: cannot open command file \"" << fileName << "\".\n";
            delete fileProcessor;
            return 1;
        }
        processor = fileProcessor;
        std::cout << "Reading commands from file \"" << fileName << "\".\n";
    }

    while (!engine.isFinished()) {
        triggerGameEvents(engine);

        if (useConsole) {
            std::cout << "\nState: " << engine.getCurrentStateName() << "\nValid commands:";
            for (const std::string& name : engine.getValidCommands()) {
                std::cout << " " << name;
            }
            std::cout << "\n> ";
        }

        Command* command = processor->getCommand();
        if (command == nullptr) {
            std::cout << "\nNo more commands.\n";
            break;
        }

        if (processor->validate(command)) {
            engine.processCommand(command->getName());
            command->saveEffect("Accepted. Game state is now [" + engine.getCurrentStateName() + "].");
        }
        std::cout << *command << "\n";
    }

    std::cout << "\n===== All commands saved by the processor =====\n"
              << *processor << "\n";

    delete processor;
    return 0;
}
