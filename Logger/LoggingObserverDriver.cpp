#include "LoggingObserver.h"
#include "../GameEngine/GameEngine.h"
#include "../CommandProcessing/CommandProcessing.h"
#include "../Orders/Orders.h"

#include <fstream>
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

/**
 * Reads commands from the console (-console) or a file (-file <filename>) and
 * applies them to the game engine. The LogObserver is attached to the engine
 * and the command processor so that every command, its effect and every state
 * change is written to gamelog.txt.
 */
int commandProcessingWithLog(int argc, char* argv[], LogObserver* logObserver) {
    std::string program = argc > 0 ? argv[0] : "loggingobserver";
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
    engine.attach(logObserver); // log every state change
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
    processor->attach(logObserver); // log every command and its effect

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

/**
 * Runs the game startup phase with the LogObserver attached to the game engine
 * and its command processor, so that the startup commands and state changes
 * are written to gamelog.txt.
 */
void startupPhaseWithLog(LogObserver* logObserver) {
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
    engine.attach(logObserver); // log every state change
    if (engine.getCommandProcessor() != nullptr)
    {
        engine.getCommandProcessor()->attach(logObserver); // log every command and its effect
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
}

/**
 * Adds two orders to an orders list and executes them, so that both the
 * issued orders and the executed orders are written to gamelog.txt.
 */
void ordersWithLog(LogObserver* logObserver) {
    OrdersList orders;
    orders.attach(logObserver); // log every order added

    Orders* deploy = new Deploy();
    Orders* bomb = new Bomb();
    deploy->attach(logObserver); // log the order when it is executed
    bomb->attach(logObserver);

    orders.add(deploy);
    orders.add(bomb);
    orders.execute();
}

} // namespace

/**
 * Driver for Part 5. Shows that commands, command effects, state changes,
 * issued orders and executed orders are all written to gamelog.txt through
 * the Observer pattern, then prints the log file.
 */
int main(int argc, char* argv[]) {
    // start with an empty log file for this run
    std::ofstream clearFile("gamelog.txt");
    clearFile.close();

    LogObserver logObserver;

    commandProcessingWithLog(argc, argv, &logObserver);
    startupPhaseWithLog(&logObserver);
    ordersWithLog(&logObserver);

    std::cout << "\n--- Contents of gamelog.txt ---\n";
    std::ifstream logFile("gamelog.txt");
    std::string line;
    while (std::getline(logFile, line)) {
        std::cout << line << "\n";
    }
    return 0;
}
