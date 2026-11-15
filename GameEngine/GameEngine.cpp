#include "GameEngine.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <chrono>
#include <random>
#include <sstream>

#include <filesystem>

namespace {

/**
 * Returns the index of 'target' inside 'states', or -1 if it is not present.
 * Used to re-link transitions when a GameEngine is copied.
 */
int indexOfState(const std::vector<State*>& states, const State* target) {
    for (size_t i = 0; i < states.size(); ++i) {
        if (states[i] == target) {
            return static_cast<int>(i);
        }
    }
    return -1;
}



    // normalize a command string (trim whitespace and convert to lowercase)
    std::string normalizeCommand(const std::string &text)
    {
        size_t first = 0;
        size_t last = text.size();
        while (first < last && std::isspace(static_cast<unsigned char>(text[first])))
        {
            ++first;
        }
        while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1])))
        {
            --last;
        }
        std::string result = text.substr(first, last - first);
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c)
                       { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    // SPLITTING FULL COMMAND INTO BASE COMMAND AND ARGUMENT
    void splitCommandAndArg(const std::string &fullCommand, std::string &baseCmd, std::string &arg)
    {
        size_t first = 0;
        while (first < fullCommand.size() && std::isspace(static_cast<unsigned char>(fullCommand[first])))
        {
            ++first;
        }
        size_t spacePos = first;
        while (spacePos < fullCommand.size() && !std::isspace(static_cast<unsigned char>(fullCommand[spacePos])))
        {
            ++spacePos;
        }

        baseCmd = normalizeCommand(fullCommand.substr(first, spacePos - first));

        size_t argStart = spacePos;
        while (argStart < fullCommand.size() && std::isspace(static_cast<unsigned char>(fullCommand[argStart])))
        {
            ++argStart;
        }
        size_t argEnd = fullCommand.size();
        while (argEnd > argStart && std::isspace(static_cast<unsigned char>(fullCommand[argEnd - 1])))
        {
            --argEnd;
        }
        arg = fullCommand.substr(argStart, argEnd - argStart);
    }

    /** Lists available .map files in the current directory and ./maps (if it exists). */
    std::vector<std::string> findAvailableMapFiles()
    {
        std::vector<std::string> mapFiles;
        const std::vector<std::string> searchDirs = {".", "maps"};
        for (const std::string &dir : searchDirs)
        {
            std::error_code ec;
            if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec))
            {
                continue;
            }
            for (const auto &entry : std::filesystem::directory_iterator(dir, ec))
            {
                if (entry.is_regular_file(ec) && entry.path().extension() == ".map")
                {
                    std::string pathStr = entry.path().lexically_normal().string();
                    if (std::find(mapFiles.begin(), mapFiles.end(), pathStr) == mapFiles.end())
                    {
                        mapFiles.push_back(pathStr);
                    }
                }
            }
        }
        std::sort(mapFiles.begin(), mapFiles.end());
        return mapFiles;
    }

} // namespace

// ---------------------------------------------------------------- Transition

Transition::Transition(const std::string &command, State *target)
    : command(command), target(target) {}

Transition::Transition(const Transition &other)
    : command(other.command), target(other.target) {}

Transition &Transition::operator=(const Transition &other)
{
    if (this != &other)
    {
        command = other.command;
        target = other.target;
    }
    return *this;
}

Transition::~Transition() {}

std::string Transition::getCommand() const
{
    return command;
}

State *Transition::getTarget() const
{
    return target;
}

void Transition::setTarget(State *newTarget)
{
    target = newTarget;
}

std::ostream &operator<<(std::ostream &out, const Transition &transition)
{
    out << "--" << transition.command << "--> ";
    if (transition.target != nullptr)
    {
        out << transition.target->getName();
    }
    else
    {
        out << "(no target)";
    }
    return out;
}
// --------------------------------------------------------------------- State

State::State(const std::string &name) : name(name) {}

State::State(const State &other) : name(other.name)
{
    for (const Transition *transition : other.transitions)
    {
        transitions.push_back(new Transition(*transition));
    }
}

State &State::operator=(const State &other)
{
    if (this != &other)
    {
        clearTransitions();
        name = other.name;
        for (const Transition *transition : other.transitions)
        {
            transitions.push_back(new Transition(*transition));
        }
    }
    return *this;
}

State::~State()
{
    clearTransitions();
}

std::string State::getName() const
{
    return name;
}

void State::addTransition(const std::string &command, State *target)
{
    transitions.push_back(new Transition(command, target));
}

Transition *State::findTransition(const std::string &command) const
{
    for (Transition *transition : transitions)
    {
        if (transition->getCommand() == command)
        {
            return transition;
        }
    }
    return nullptr;
}

const std::vector<Transition *> &State::getTransitions() const
{
    return transitions;
}

bool State::isTerminal() const
{
    return transitions.empty();
}

void State::clearTransitions()
{
    for (Transition *transition : transitions)
    {
        delete transition;
    }
    transitions.clear();
}

std::ostream &operator<<(std::ostream &out, const State &state)
{
    out << "State [" << state.name << "]";
    if (state.transitions.empty())
    {
        out << " (final state)";
    }
    for (const Transition *transition : state.transitions)
    {
        out << "\n    " << *transition;
    }
    return out;
}

// ---------------------------------------------------------------- GameEngine

GameEngine::GameEngine()
    : currentState(nullptr),
      map(nullptr),
      deck(new Deck()),
      players(new std::vector<Player *>()),
      commandProcessor(new CommandProcessor(this)),
      ownsCommandProcessor(true)
{
    buildStateGraph();
}

GameEngine::GameEngine(CommandProcessor *processor)
    : currentState(nullptr),
      map(nullptr),
      deck(new Deck()),
      players(new std::vector<Player *>()),
      commandProcessor(processor != nullptr ? processor : new CommandProcessor(this)),
      ownsCommandProcessor(processor == nullptr)
{
    buildStateGraph();
}

GameEngine::GameEngine(const GameEngine &other)
    : Subject(other),
      ILoggable(other),
      currentState(nullptr),
      map(nullptr),
      deck(nullptr),
      players(nullptr),
      commandProcessor(nullptr),
      ownsCommandProcessor(false)
{
    copyFrom(other);
}

GameEngine &GameEngine::operator=(const GameEngine &other)
{
    if (this != &other)
    {
        clear();
        copyFrom(other);
    }
    return *this;
}

GameEngine::~GameEngine()
{
    clear();
}

State *GameEngine::addState(const std::string &name)
{
    State *state = new State(name);
    states.push_back(state);
    return state;
}

// updated transition command names to match a2
void GameEngine::buildStateGraph()
{
    State *start = addState("start");
    State *mapLoaded = addState("map loaded");
    State *mapValidated = addState("map validated");
    State *playersAdded = addState("players added");
    State *assignReinforcement = addState("assign reinforcement");
    State *issueOrders = addState("issue orders");
    State *executeOrders = addState("execute orders");
    State *win = addState("win");
    State *exitState = addState("exit");

    // changed command names
    start->addTransition("loadmap", mapLoaded);
    mapLoaded->addTransition("loadmap", mapLoaded);
    mapLoaded->addTransition("validatemap", mapValidated);
    mapValidated->addTransition("addplayer", playersAdded);
    playersAdded->addTransition("addplayer", playersAdded);
    playersAdded->addTransition("gamestart", assignReinforcement);

    // changed command names
    assignReinforcement->addTransition("issueorder", issueOrders);
    issueOrders->addTransition("issueorder", issueOrders);
    issueOrders->addTransition("endissueorders", executeOrders);
    executeOrders->addTransition("execorder", executeOrders);
    executeOrders->addTransition("endexecorders", assignReinforcement);
    executeOrders->addTransition("win", win);
    win->addTransition("replay", start);
    win->addTransition("quit", exitState);

    currentState = start;
}

// updated for a2 pointers
void GameEngine::copyFrom(const GameEngine &other)
{
    for (const State *state : other.states)
    {
        states.push_back(new State(*state));
    }

    for (State *state : states)
    {
        for (Transition *transition : state->getTransitions())
        {
            int index = indexOfState(other.states, transition->getTarget());
            transition->setTarget(index >= 0 ? states[index] : nullptr);
        }
    }

    int currentIndex = indexOfState(other.states, other.currentState);
    currentState = currentIndex >= 0 ? states[currentIndex] : nullptr;

    map = (other.map != nullptr) ? new Map(*other.map) : nullptr;
    deck = (other.deck != nullptr) ? new Deck(*other.deck) : nullptr;

    players = new std::vector<Player *>();
    if (other.players != nullptr)
    {
        for (const Player *player : *(other.players))
        {
            players->push_back(new Player(*player));
        }
    }

    if (other.commandProcessor != nullptr)
    {
        commandProcessor = new CommandProcessor(*other.commandProcessor);
        ownsCommandProcessor = true;
    }
    else
    {
        commandProcessor = nullptr;
        ownsCommandProcessor = false;
    }
}
// updated for a2 pointers
void GameEngine::clear()
{
    for (State *state : states)
    {
        delete state;
    }
    states.clear();
    currentState = nullptr;
    delete map;
    map = nullptr;
    delete deck;
    deck = nullptr;
    if (players != nullptr)
    {
        for (Player *player : *players)
        {
            delete player;
        }
        delete players;
        players = nullptr;
    }
    if (ownsCommandProcessor && commandProcessor != nullptr)
    {
        delete commandProcessor;
    }
    commandProcessor = nullptr;
    ownsCommandProcessor = false;
}
// changes state and notifies logobserver
void GameEngine::transition(State *newState)
{
    if (newState == nullptr)
    {
        return;
    }
    State *previous = currentState;
    currentState = newState;
    std::cout << "[" << (previous ? previous->getName() : "none") << "] --> ["
              << currentState->getName() << "]" << std::endl;
    notify(this);
}
/**
 * Processes a raw command string, validates it against the current state's
 * transitions, and updates the state if valid.
 */
bool GameEngine::processCommand(const std::string &command)
{
    if (currentState == nullptr)
    {
        std::cout << "Error: GameEngine has no active state." << std::endl;
        return false;
    }

    std::string cleaned = normalizeCommand(command);
    Transition *nextTransition = currentState->findTransition(cleaned);
    if (nextTransition == nullptr)
    {
        std::cout << "Error: command \"" << cleaned
                  << "\" is not valid in state ["
                  << currentState->getName() << "]. No state change." << std::endl;
        return false;
    }

    transition(nextTransition->getTarget());
    return true;
}

/** Returns a pointer to the current State object. */
const State *GameEngine::getCurrentState() const
{
    return currentState;
}

/** Returns the name of the current state, or "none" if unset. */
std::string GameEngine::getCurrentStateName() const
{
    return (currentState != nullptr) ? currentState->getName() : "none";
}

/** Returns a list of valid command strings available in the current state. */
std::vector<std::string> GameEngine::getValidCommands() const
{
    std::vector<std::string> commands;
    if (currentState != nullptr)
    {
        for (const Transition *transition : currentState->getTransitions())
        {
            commands.push_back(transition->getCommand());
        }
    }
    return commands;
}

/** Returns true if the engine has reached a terminal state with no outgoing transitions. */
bool GameEngine::isFinished() const
{
    return (currentState != nullptr) ? currentState->isTerminal() : true;
}

/** Stream insertion operator printing the current state and full state graph. */
std::ostream &operator<<(std::ostream &out, const GameEngine &engine)
{
    out << "GameEngine - current state: "
        << (engine.currentState != nullptr ? engine.currentState->getName() : "none") << "\n";
    for (const State *state : engine.states)
    {
        if (state != nullptr)
        {
            out << *state << "\n";
        }
    }
    return out;
}

/** Returns the log line describing the new state of the engine. */
std::string GameEngine::stringToLog() const {
    return "Game Engine New State: " + currentState->getName();
}

void GameEngine::enterPlayState(const std::string& name) {
    for (State* state : states) {
        if (state->getName() == name) { transition(state); return; }
    }
    throw std::logic_error("Unknown gameplay state.");
}

void GameEngine::reinforcementPhase(const Map& gameMap, const std::vector<Player*>& activePlayers) {
    enterPlayState("assign reinforcement");
    for (Player* player : activePlayers) {
        const auto& territories = gameMap.getCountries();
        const int owned = static_cast<int>(std::count_if(territories.begin(), territories.end(),
            [player](Territory* t) { return t->getOwner() == player; }));
        if (owned == 0) continue;
        int award = std::max(3, owned / 3);
        for (Continent* continent : gameMap.getContinents()) {
            const auto& members = continent->getTerritories();
            if (!members.empty() && std::all_of(members.begin(), members.end(),
                    [player](Territory* t) { return t->getOwner() == player; }))
                award += continent->getBonus();
        }
        player->addReinforcements(award);
    }
}

void GameEngine::issueOrdersPhase(const std::vector<Player*>& activePlayers, Deck* gameDeck) {
    // Never silently reset reservations while old orders are still pending.
    for (size_t i = 0; i < activePlayers.size(); ++i) {
        OrdersList* list = activePlayers[i]->getOrdersList();
        if (!list || list->getOrdersListSize() != 0)
            throw std::logic_error("Each active player needs an empty OrdersList.");
        for (size_t j = 0; j < i; ++j)
            if (list == activePlayers[j]->getOrdersList())
                throw std::logic_error("Active players must have separate order lists.");
    }
    enterPlayState("issue orders");
    std::vector<int> remainingArmies, nextOrder(activePlayers.size(), 0);
    for (Player* player : activePlayers) remainingArmies.push_back(player->getReinforcementPool());
    std::vector<bool> finished(activePlayers.size(), false);
    size_t remaining = activePlayers.size();
    while (remaining > 0) {
        for (size_t i = 0; i < activePlayers.size(); ++i) {
            if (!finished[i] && !activePlayers[i]->issueOrder(gameDeck, remainingArmies[i], nextOrder[i])) {
                finished[i] = true;
                --remaining;
            }
        }
    }
}

void GameEngine::executeOrdersPhase(const Map& gameMap, std::vector<Player*>& activePlayers) {
    if (checkVictory(gameMap, activePlayers)) return;
    enterPlayState("execute orders");
    // One matching order per player per sweep. Pass 0 selects Deploy even if
    // it was moved behind another type; pass 1 consumes the remaining fronts.
    for (int pass = 0; pass < 2; ++pass) {
        bool executed;
        do {
            executed = false;
            for (Player* player : activePlayers) {
                OrdersList* list = player->getOrdersList();
                if (!list) throw std::logic_error("Active player has no OrdersList.");
                for (size_t i = 0; i < list->getOrdersListSize(); ++i) {
                    Orders* order = list->getOrder(static_cast<int>(i));
                    if (pass == 0 && order->getName() != "Deploy") continue;
                    std::cout << player->getName() << " executes " << order->getName() << '\n';
                    order->execute(); // Part 4 performs validation and effects.
                    list->remove(static_cast<int>(i)); // Deletes the order.
                    if (checkVictory(gameMap, activePlayers)) {
                        eliminatePlayers(gameMap, activePlayers);
                        return;
                    }
                    executed = true;
                    break;
                }
            }
        } while (executed);
    }
    eliminatePlayers(gameMap, activePlayers);
    enterPlayState("assign reinforcement");
}

void GameEngine::eliminatePlayers(const Map& gameMap, std::vector<Player*>& activePlayers) {
    const auto& territories = gameMap.getCountries();
    activePlayers.erase(std::remove_if(activePlayers.begin(), activePlayers.end(), [&](Player* player) {
        return std::none_of(territories.begin(), territories.end(),
            [player](Territory* t) { return t->getOwner() == player; });
    }), activePlayers.end()); // Erase participation only, never delete Player.
}

Player* GameEngine::checkVictory(const Map& gameMap, const std::vector<Player*>& activePlayers) {
    const auto& territories = gameMap.getCountries();
    if (territories.empty()) return nullptr;
    Player* winner = territories.front()->getOwner();
    if (!winner || std::find(activePlayers.begin(), activePlayers.end(), winner) == activePlayers.end()) return nullptr;
    if (!std::all_of(territories.begin(), territories.end(),
            [winner](Territory* t) { return t->getOwner() == winner; })) return nullptr;
    if (getCurrentStateName() != "win") {
        enterPlayState("win");
        std::cout << "Winner: " << winner->getName() << '\n';
    }
    return winner;
}

void GameEngine::mainGameLoop(const Map& gameMap, std::vector<Player*>& activePlayers, Deck* gameDeck) {
    if (gameMap.getCountries().empty()) throw std::logic_error("Cannot play an empty map.");
    eliminatePlayers(gameMap, activePlayers);
    if (activePlayers.empty()) throw std::logic_error("No active players own territories.");
    // Do not run a full game with the current no-effect Part 4 placeholders.
    // There is deliberately no turn limit or artificial victory condition.
    while (!checkVictory(gameMap, activePlayers)) {
        reinforcementPhase(gameMap, activePlayers);
        issueOrdersPhase(activePlayers, gameDeck);
        executeOrdersPhase(gameMap, activePlayers);
    }
}


// -------------------------------------------- Getters & Setters for Startup Phase

/** Returns the loaded Map pointer. */
Map *GameEngine::getMap() const
{
    return map;
}

/** Returns the game Deck pointer. */
Deck *GameEngine::getDeck() const
{
    return deck;
}

/** Returns the pointer to the vector of active Players. */
std::vector<Player *> *GameEngine::getPlayers() const
{
    return players;
}

/** Returns the active CommandProcessor pointer. */
CommandProcessor *GameEngine::getCommandProcessor() const
{
    return commandProcessor;
}

/** Sets an external CommandProcessor without taking ownership of its memory. */
void GameEngine::setCommandProcessor(CommandProcessor *processor)
{
    if (ownsCommandProcessor && commandProcessor != nullptr)
    {
        delete commandProcessor;
    }
    commandProcessor = processor;
    ownsCommandProcessor = false;
}


void GameEngine::startupPhase()
{
    if (commandProcessor == nullptr)
    {
        std::cout << "Error: No CommandProcessor attached to GameEngine." << std::endl;
        return;
    }
    std::cout << "\n=== Game Startup Phase ===" << std::endl;
    while (currentState != nullptr && currentState->getName() != "assign reinforcement" && !isFinished())
    {
        std::cout << "\nCurrent state: [" << currentState->getName() << "]" << std::endl;
        std::cout << "Valid commands:";
        for (const std::string &validCmd : getValidCommands())
        {
            std::cout << " " << validCmd;
        }
        std::cout << std::endl;
        if (currentState->getName() == "start" || currentState->getName() == "map loaded")
        {
            std::vector<std::string> availableMaps = findAvailableMapFiles();
            std::cout << "Available map files in directory:";
            if (availableMaps.empty())
            {
                std::cout << " (none found in '.' or './maps')";
            }
            else
            {
                for (const std::string &mapFile : availableMaps)
                {
                    std::cout << " " << mapFile;
                }
            }
            std::cout << std::endl;
        }
        Command *cmd = commandProcessor->getCommand();
        if (cmd == nullptr)
        {
            std::cout << "No more commands available. Ending startup phase." << std::endl;
            break;
        } // Validate command against current game state
        if (!commandProcessor->validate(cmd))
        {
            std::cout << "Invalid command in state [" << currentState->getName() << "]." << std::endl;
            continue;
        }
        std::string baseCmd;
        std::string arg;
        splitCommandAndArg(cmd->getText(), baseCmd, arg);
        Transition *nextTransition = currentState->findTransition(baseCmd);
        if (nextTransition == nullptr)
        {
            cmd->saveEffect("Error: No state transition found for command '" + baseCmd + "'.");
            continue;
        }
        std::string effect;
        bool success = false;
        if (baseCmd == "loadmap")
        {
            success = handleLoadMap(arg, effect);
        }
        else if (baseCmd == "validatemap")
        {
            success = handleValidateMap(effect);
        }
        else if (baseCmd == "addplayer")
        {
            success = handleAddPlayer(arg, effect);
        }
        else if (baseCmd == "gamestart")
        {
            success = handleGameStart(effect);
        }
        else
        {
            effect = "Error: Unsupported startup command '" + baseCmd + "'.";
        }
        cmd->saveEffect(effect);
        std::cout << effect << std::endl;
        if (success)
        {
            transition(nextTransition->getTarget());
        }
    }
}


/**
• Handles "loadmap ":
• Loads the requested map using MapLoader.
*/
bool GameEngine::handleLoadMap(const std::string &filename, std::string &effectOut)
{
    if (filename.empty())
    {
        effectOut = "Error: loadmap requires a  argument.";
        return false;
    } // Check direct path first, then fallback to maps/
    std::string resolvedPath = filename;
    std::error_code ec;
    if (!std::filesystem::exists(resolvedPath, ec))
    {
        std::filesystem::path fallback = std::filesystem::path("maps") / filename;
        if (std::filesystem::exists(fallback, ec))
        {
            resolvedPath = fallback.string();
        }
    }
    MapLoader loader;
    Map *loadedMap = nullptr;
    try
    {
        loadedMap = loader.loadMap(resolvedPath);
    }
    catch (const std::exception &ex)
    {
        effectOut = "Error: Failed to load map '" + filename + "' (" + ex.what() + ").";
        return false;
    }
    if (loadedMap == nullptr)
    {
        effectOut = "Error: Failed to load map '" + filename + "'.";
        return false;
    } // Replace any previously loaded map without leaking memory
    delete map;
    map = loadedMap;
    effectOut = "Map '" + resolvedPath + "' loaded successfully (" + std::to_string(map->getContinents().size()) + " continents, " + std::to_string(map->getCountries().size()) + " territories).";
    return true;
}



/**
• Handles "validatemap":
• Checks that a map is loaded and passes Map::validate() (connected graph,
• connected continents, each territory belongs to exactly one continent).
*/
bool GameEngine::handleValidateMap(std::string &effectOut)
{
    if (map == nullptr)
    {
        effectOut = "Error: No map is currently loaded to validate.";
        return false;
    }
    if (!map->validate())
    {
        effectOut = "Error: Map validation failed (map is not a valid connected graph).";
        return false;
    }
    effectOut = "Map validated successfully.";
    return true;
}



/**
• Handles "addplayer ":
• Adds a player to the game (enforcing 2-6 players total) and attaches
• their Hand and OrdersList so they are ready for gameplay.
*/
bool GameEngine::handleAddPlayer(const std::string &playerName, std::string &effectOut)
{
    if (playerName.empty())
    {
        effectOut = "Error: addplayer requires a  argument.";
        return false;
    }
    if (players == nullptr)
    {
        players = new std::vector<Player *>();
    }
    if (players->size() >= 6)
    {
        effectOut = "Error: Maximum of 6 players already reached. Cannot add '" + playerName + "'.";
        return false;
    }
    for (const Player *existing : *players)
    {
        if (existing != nullptr && existing->getName() == playerName)
        {
            effectOut = "Error: Player '" + playerName + "' already exists in the game.";
            return false;
        }
    }
    Player *newPlayer = new Player(playerName);
    newPlayer->setHand(new Hand());
    newPlayer->setOrdersList(new OrdersList());
    players->push_back(newPlayer);
    effectOut = "Player '" + playerName + "' added (" + std::to_string(players->size()) + "/6 players).";
    return true;
}

/**
 * Handles "gamestart":
 * 4a) Fairly distributes all territories to the players
 * 4b) Randomly determines the order of play of the players
 * 4c) Gives 50 initial armies to each player's reinforcement pool
 * 4d) Lets each player draw 2 initial cards from the deck using Deck::draw()
 * 4e) Signals readiness to transition into the play phase ("assign reinforcement")
 */
bool GameEngine::handleGameStart(std::string &effectOut)
{
    if (map == nullptr)
    {
        effectOut = "Error: Cannot start game without a loaded and validated map.";
        return false;
    }

    if (players == nullptr || players->size() < 2 || players->size() > 6)
    {
        size_t count = (players != nullptr) ? players->size() : 0;
        effectOut = "Error: Game requires 2 to 6 players to start (currently " + std::to_string(count) + ").";
        return false;
    }

    // 4b) Determine randomly the order of play of the players in the game
    unsigned int seed = static_cast<unsigned int>(
        std::chrono::system_clock::now().time_since_epoch().count());
    std::default_random_engine rng(seed);
    std::shuffle(players->begin(), players->end(), rng);

    // 4a) Fairly distribute all the territories to the players (round-robin)
    std::vector<Territory *> allTerritories = map->getCountries();
    std::shuffle(allTerritories.begin(), allTerritories.end(), rng);
    for (size_t i = 0; i < allTerritories.size(); ++i)
    {
        Player *assignedPlayer = (*players)[i % players->size()];
        assignedPlayer->addTerritory(allTerritories[i]);
    }

    // Ensure the Deck is populated with cards before drawing
    if (deck == nullptr)
    {
        deck = new Deck();
    }
    if (deck->getSize() == 0)
    {
        const CardType allTypes[] = {
            CardType::Bomb,
            CardType::Reinforcement,
            CardType::Blockade,
            CardType::Airlift,
            CardType::Diplomacy};

        // Populate 30 cards (6 of each type) so up to 6 players can draw 2 cards each
        for (int copy = 0; copy < 6; ++copy)
        {
            for (CardType type : allTypes)
            {
                deck->addCard(new Card(type));
            }
        }
    }

    // 4c) Give 50 initial armies to each player & 4d) Draw 2 initial cards
    const int INITIAL_ARMIES = 50;

    std::ostringstream summary;
    summary << "Game started! Play order:";
    for (size_t i = 0; i < players->size(); ++i)
    {
        Player *p = (*players)[i];

        // Ensure Hand and OrdersList are attached in case a player was added externally
        if (p->getHand() == nullptr)
        {
            p->setHand(new Hand());
        }
        if (p->getOrdersList() == nullptr)
        {
            p->setOrdersList(new OrdersList());
        }

        // 4c) 50 initial armies in reinforcement pool
        p->addReinforcements(INITIAL_ARMIES);

        // 4d) Let each player draw 2 initial cards from the deck using Deck::draw()
        deck->draw(*p->getHand());
        deck->draw(*p->getHand());

        summary << "\n  " << (i + 1) << ". " << p->getName()
                << " | Territories: " << p->toDefend().size()
                << " | Initial Armies: " << INITIAL_ARMIES
                << " | Cards in Hand: " << p->getHand()->getSize();
    }

    // 4e) Returning true triggers transition("assign reinforcement") in startupPhase()
    effectOut = summary.str();
    return true;
}
