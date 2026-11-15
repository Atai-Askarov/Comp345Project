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
      commandProcessor(new CommandProcessor()),
      ownsCommandProcessor(true)
{
    buildStateGraph();
}

GameEngine::GameEngine(CommandProcessor *processor)
    : currentState(nullptr),
      map(nullptr),
      deck(new Deck()),
      players(new std::vector<Player *>()),
      commandProcessor(processor != nullptr ? processor : new CommandProcessor()),
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

    State *previous = currentState;
    currentState = transition(nextTransition->getTarget());
    std::cout << "[" << previous->getName() << "] --" << cleaned << "--> ["
              << (currentState != nullptr ? currentState->getName() : "none") << "]" << std::endl;
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
