#include "GameEngine.h"

#include <algorithm>
#include <cctype>

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

/** Removes surrounding whitespace and converts the text to lower case. */
std::string normalizeCommand(const std::string& text) {
    size_t first = 0;
    size_t last = text.size();
    while (first < last && std::isspace(static_cast<unsigned char>(text[first]))) {
        ++first;
    }
    while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
        --last;
    }
    std::string result = text.substr(first, last - first);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

} // namespace

// ---------------------------------------------------------------- Transition

Transition::Transition(const std::string& command, State* target)
    : command(command), target(target) {}

Transition::Transition(const Transition& other)
    : command(other.command), target(other.target) {}

Transition& Transition::operator=(const Transition& other) {
    if (this != &other) {
        command = other.command;
        target = other.target;
    }
    return *this;
}

Transition::~Transition() {}

std::string Transition::getCommand() const {
    return command;
}

State* Transition::getTarget() const {
    return target;
}

void Transition::setTarget(State* newTarget) {
    target = newTarget;
}

std::ostream& operator<<(std::ostream& out, const Transition& transition) {
    out << "--" << transition.command << "--> ";
    if (transition.target != nullptr) {
        out << transition.target->getName();
    } else {
        out << "(no target)";
    }
    return out;
}

// --------------------------------------------------------------------- State

State::State(const std::string& name) : name(name) {}

State::State(const State& other) : name(other.name) {
    for (const Transition* transition : other.transitions) {
        transitions.push_back(new Transition(*transition));
    }
}

State& State::operator=(const State& other) {
    if (this != &other) {
        clearTransitions();
        name = other.name;
        for (const Transition* transition : other.transitions) {
            transitions.push_back(new Transition(*transition));
        }
    }
    return *this;
}

State::~State() {
    clearTransitions();
}

std::string State::getName() const {
    return name;
}

void State::addTransition(const std::string& command, State* target) {
    transitions.push_back(new Transition(command, target));
}

Transition* State::findTransition(const std::string& command) const {
    for (Transition* transition : transitions) {
        if (transition->getCommand() == command) {
            return transition;
        }
    }
    return nullptr;
}

const std::vector<Transition*>& State::getTransitions() const {
    return transitions;
}

bool State::isTerminal() const {
    return transitions.empty();
}

void State::clearTransitions() {
    for (Transition* transition : transitions) {
        delete transition;
    }
    transitions.clear();
}

std::ostream& operator<<(std::ostream& out, const State& state) {
    out << "State [" << state.name << "]";
    if (state.transitions.empty()) {
        out << " (final state)";
    }
    for (const Transition* transition : state.transitions) {
        out << "\n    " << *transition;
    }
    return out;
}

// ---------------------------------------------------------------- GameEngine

GameEngine::GameEngine() : currentState(nullptr) {
    buildStateGraph();
}

GameEngine::GameEngine(const GameEngine& other) : currentState(nullptr) {
    copyFrom(other);
}

GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        clear();
        copyFrom(other);
    }
    return *this;
}

GameEngine::~GameEngine() {
    clear();
}

State* GameEngine::addState(const std::string& name) {
    State* state = new State(name);
    states.push_back(state);
    return state;
}

void GameEngine::buildStateGraph() {
    State* start = addState("start");
    State* mapLoaded = addState("map loaded");
    State* mapValidated = addState("map validated");
    State* playersAdded = addState("players added");
    State* assignReinforcement = addState("assign reinforcement");
    State* issueOrders = addState("issue orders");
    State* executeOrders = addState("execute orders");
    State* win = addState("win");
    State* exitState = addState("exit");

    // Startup phase
    start->addTransition("loadmap", mapLoaded);
    mapLoaded->addTransition("loadmap", mapLoaded);
    mapLoaded->addTransition("validatemap", mapValidated);
    mapValidated->addTransition("addplayer", playersAdded);
    playersAdded->addTransition("addplayer", playersAdded);
    playersAdded->addTransition("assigncountries", assignReinforcement);

    // Play phase
    assignReinforcement->addTransition("issueorder", issueOrders);
    issueOrders->addTransition("issueorder", issueOrders);
    issueOrders->addTransition("endissueorders", executeOrders);
    executeOrders->addTransition("execorder", executeOrders);
    executeOrders->addTransition("endexecorders", assignReinforcement);
    executeOrders->addTransition("win", win);
    win->addTransition("play", start);
    win->addTransition("end", exitState);

    currentState = start;
}

void GameEngine::copyFrom(const GameEngine& other) {
    // Copy every state; the copied transitions still point at other's states.
    for (const State* state : other.states) {
        states.push_back(new State(*state));
    }

    // Re-link every copied transition to the matching state of this engine.
    for (State* state : states) {
        for (Transition* transition : state->getTransitions()) {
            int index = indexOfState(other.states, transition->getTarget());
            transition->setTarget(index >= 0 ? states[index] : nullptr);
        }
    }

    int currentIndex = indexOfState(other.states, other.currentState);
    currentState = currentIndex >= 0 ? states[currentIndex] : nullptr;
}

void GameEngine::clear() {
    for (State* state : states) {
        delete state;
    }
    states.clear();
    currentState = nullptr;
}

bool GameEngine::processCommand(const std::string& command) {
    std::string cleaned = normalizeCommand(command);
    Transition* transition = currentState->findTransition(cleaned);

    if (transition == nullptr) {
        std::cout << "Error: command \"" << cleaned << "\" is not valid in state ["
                  << currentState->getName() << "]. No state change." << std::endl;
        return false;
    }

    State* previous = currentState;
    currentState = transition->getTarget();
    std::cout << "[" << previous->getName() << "] --" << cleaned << "--> ["
              << currentState->getName() << "]" << std::endl;
    return true;
}

const State* GameEngine::getCurrentState() const {
    return currentState;
}

std::string GameEngine::getCurrentStateName() const {
    return currentState->getName();
}

std::vector<std::string> GameEngine::getValidCommands() const {
    std::vector<std::string> commands;
    for (const Transition* transition : currentState->getTransitions()) {
        commands.push_back(transition->getCommand());
    }
    return commands;
}

bool GameEngine::isFinished() const {
    return currentState->isTerminal();
}

std::ostream& operator<<(std::ostream& out, const GameEngine& engine) {
    out << "GameEngine - current state: " << engine.currentState->getName() << "\n";
    for (const State* state : engine.states) {
        out << *state << "\n";
    }
    return out;
}
