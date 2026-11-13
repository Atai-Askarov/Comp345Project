#include "GameEngine.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>
#include <utility>
#include "../Map/Map.h"
#include "../Player/Player.h"
#include "../Orders/Orders.h"

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
    try {
        for (const Transition* transition : other.transitions) {
            Transition* copy = new Transition(*transition);
            try { transitions.push_back(copy); }
            catch (...) { delete copy; throw; }
        }
    } catch (...) {
        clearTransitions();
        throw;
    }
}

State& State::operator=(const State& other) {
    if (this != &other) {
        State copy(other);
        name.swap(copy.name);
        transitions.swap(copy.transitions);
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
    Transition* transition = new Transition(command, target);
    try { transitions.push_back(transition); }
    catch (...) { delete transition; throw; }
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

GameEngine::GameEngine() : currentState(nullptr), gameMap(nullptr), gameDeck(nullptr), winner(nullptr) {
    try { buildStateGraph(); }
    catch (...) { clear(); throw; }
}

GameEngine::GameEngine(const GameEngine& other)
    : currentState(nullptr), gameMap(nullptr), gameDeck(nullptr), winner(nullptr) {
    try { copyFrom(other); }
    catch (...) { clear(); throw; }
}

GameEngine& GameEngine::operator=(const GameEngine& other) {
    if (this != &other) {
        GameEngine copy(other);
        states.swap(copy.states);
        std::swap(currentState, copy.currentState);
        std::swap(gameMap, copy.gameMap);
        std::swap(gameDeck, copy.gameDeck);
        activePlayers.swap(copy.activePlayers);
        std::swap(winner, copy.winner);
    }
    return *this;
}

GameEngine::~GameEngine() {
    clear();
}

State* GameEngine::addState(const std::string& name) {
    State* state = new State(name);
    try { states.push_back(state); }
    catch (...) { delete state; throw; }
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
    gameMap = other.gameMap;
    gameDeck = other.gameDeck;
    activePlayers = other.activePlayers;
    winner = other.winner;
    // Copy every state; the copied transitions still point at other's states.
    for (const State* state : other.states) {
        State* copy = new State(*state);
        try { states.push_back(copy); }
        catch (...) { delete copy; throw; }
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
    activePlayers.clear();
    gameMap = nullptr;
    gameDeck = nullptr;
    winner = nullptr;
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
    this->transition(transition->getTarget()->getName());
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
    out << "GameEngine - current state: " << engine.currentState->getName()
        << " | active players: " << engine.activePlayers.size();
    if (engine.winner) out << " | winner: " << engine.winner->getName();
    out << "\n";
    for (const State* state : engine.states) {
        out << *state << "\n";
    }
    return out;
}

void GameEngine::setGameContext(Map* map, const std::vector<Player*>& players, Deck* deck) {
    if (!map) throw std::invalid_argument("Gameplay requires a Map.");
    for (size_t i = 0; i < players.size(); ++i) {
        if (!players[i] || std::find(players.begin(), players.begin() + i, players[i])
                != players.begin() + i) {
            throw std::invalid_argument("The startup roster contains a null or duplicate player.");
        }
    }
    // Copy before replacing the live context, in case allocation fails.
    std::vector<Player*> ordered(players);
    activePlayers.swap(ordered);
    gameMap = map;
    gameDeck = deck;
    winner = nullptr;
}

Map* GameEngine::getMap() const { return gameMap; }
Deck* GameEngine::getDeck() const { return gameDeck; }
const std::vector<Player*>& GameEngine::getActivePlayers() const { return activePlayers; }
Player* GameEngine::getWinner() const { return winner; }

void GameEngine::requireGameContext() const {
    if (!gameMap) throw std::logic_error("Part 2 must supply GameEngine::setGameContext().");
}

size_t GameEngine::ownedTerritories(const Player* player) const {
    size_t count = 0;
    for (const Territory* territory : gameMap->getCountries()) {
        if (territory && territory->getOwner() == player) ++count;
    }
    return count;
}

void GameEngine::transition(const std::string& stateName) {
    for (State* state : states) {
        if (state->getName() == stateName) {
            currentState = state;
            // Part 5: notify observers here after attaching Subject/ILoggable.
            return;
        }
    }
    throw std::invalid_argument("Unknown game state: " + stateName);
}

void GameEngine::reinforcementPhase() {
    requireGameContext();
    if (winner) return;
    transition("assign reinforcement");
    for (Player* player : activePlayers) {
        const size_t territories = ownedTerritories(player);
        if (territories == 0) continue;
        long long award = std::max<size_t>(3, territories / 3);
        for (const Continent* continent : gameMap->getContinents()) {
            if (!continent || continent->getTerritories().empty()) continue;
            const auto& members = continent->getTerritories();
            if (std::all_of(members.begin(), members.end(), [player](const Territory* t) {
                    return t && t->getOwner() == player;
                })) {
                award += continent->getBonus();
            }
        }
        if (award < 0 || award > std::numeric_limits<int>::max()) {
            throw std::overflow_error("Reinforcement award out of range.");
        }
        player->addReinforcements(static_cast<int>(award));
        std::cout << player->getName() << " receives " << award
                  << " armies; pool = " << player->getReinforcementPool() << '\n';
    }
}

void GameEngine::issueOrdersPhase() {
    requireGameContext();
    if (winner) return;
    eliminatePlayers();
    // Preflight every player before resetting anybody's issuing state.
    for (size_t i = 0; i < activePlayers.size(); ++i) {
        OrdersList* list = activePlayers[i]->getOrdersList();
        if (!list || list->getOrdersListSize() != 0) {
            throw std::logic_error("Each active player needs its own empty OrdersList.");
        }
        if (!activePlayers[i]->hasOrderFactory()) {
            throw std::logic_error("Part 4 must install Player::setOrderFactory() before gameplay issuance.");
        }
        for (size_t j = 0; j < i; ++j) {
            if (list == activePlayers[j]->getOrdersList()) {
                throw std::logic_error("Active players cannot share an OrdersList.");
            }
        }
    }
    transition("issue orders");
    for (Player* player : activePlayers) player->beginOrderIssuing(gameDeck);
    std::vector<bool> finished(activePlayers.size(), false);
    size_t remaining = activePlayers.size();
    while (remaining != 0) {
        for (size_t i = 0; i < activePlayers.size(); ++i) {
            if (!finished[i] && !activePlayers[i]->issueOrder()) {
                finished[i] = true;
                --remaining;
            }
        }
    }
}

void GameEngine::executeOrdersPhase() {
    requireGameContext();
    if (winner || checkVictory()) return;
    for (size_t i = 0; i < activePlayers.size(); ++i) {
        OrdersList* list = activePlayers[i]->getOrdersList();
        if (!list) throw std::logic_error("Active player has no OrdersList.");
        for (size_t j = 0; j < i; ++j) {
            if (list == activePlayers[j]->getOrdersList()) {
                throw std::logic_error("Active players cannot share an OrdersList.");
            }
        }
    }
    transition("execute orders");
    for (int pass = 0; pass < 2; ++pass) {
        bool executed;
        do {
            executed = false;
            for (Player* player : activePlayers) {
                OrdersList* list = player->getOrdersList();
                int index = -1;
                for (size_t i = 0; i < list->getOrdersListSize(); ++i) {
                    Orders* order = list->getOrder(static_cast<int>(i));
                    if (!order) throw std::logic_error("Null order in OrdersList.");
                    if (pass == 1 || order->getName() == "Deploy") {
                        index = static_cast<int>(i);
                        break;
                    }
                }
                if (index < 0) continue;
                // Part 4 validates, applies effects and logs before returning.
                // On an exception leave the order owned by its list and propagate.
                list->getOrder(index)->execute();
                list->remove(index);
                executed = true;
                if (checkVictory()) {
                    for (Player* participant : activePlayers) participant->endOrderIssuing();
                    eliminatePlayers();
                    return;
                }
            }
        } while (executed);
    }
    for (Player* player : activePlayers) player->endOrderIssuing();
    eliminatePlayers();
    transition("assign reinforcement");
}

void GameEngine::eliminatePlayers() {
    requireGameContext();
    activePlayers.erase(std::remove_if(activePlayers.begin(), activePlayers.end(),
        [this](Player* player) {
            if (ownedTerritories(player) != 0) return false;
            std::cout << player->getName() << " is eliminated.\n";
            // Only erase participation: pending orders can still borrow Player.
            player->endOrderIssuing();
            return true;
        }), activePlayers.end());
}

Player* GameEngine::checkVictory() {
    requireGameContext();
    if (winner) return winner;
    const auto& countries = gameMap->getCountries();
    if (countries.empty() || !countries.front()) return nullptr;
    Player* candidate = countries.front()->getOwner();
    if (!candidate || std::find(activePlayers.begin(), activePlayers.end(), candidate)
            == activePlayers.end()) return nullptr;
    if (!std::all_of(countries.begin(), countries.end(), [candidate](const Territory* t) {
            return t && t->getOwner() == candidate;
        })) return nullptr;
    winner = candidate;
    transition("win");
    std::cout << "Winner: " << winner->getName() << " controls every territory.\n";
    return winner;
}

void GameEngine::mainGameLoop(size_t maxTurns) {
    requireGameContext();
    if (gameMap->getCountries().empty()) throw std::logic_error("Cannot play an empty map.");
    eliminatePlayers();
    if (checkVictory()) return;
    if (activePlayers.empty()) throw std::logic_error("No active players own map territories.");
    // Fail before granting armies when known placeholder dependencies are absent.
    for (Player* player : activePlayers) {
        if (!player->hasOrderFactory()) {
            throw std::logic_error("Part 4 order factory is missing; full gameplay is unavailable.");
        }
    }
    size_t turns = 0;
    while (!winner) {
        if (maxTurns != 0 && turns >= maxTurns) {
            throw std::runtime_error("Diagnostic turn limit reached without a winner.");
        }
        reinforcementPhase();
        issueOrdersPhase();
        executeOrdersPhase();
        eliminatePlayers();
        if (checkVictory()) return;
        if (activePlayers.empty()) throw std::logic_error("No active players remain.");
        // Part 4 integration: expire negotiations and award conquest cards at
        // this boundary, before the next reinforcement. No effects are faked.
        ++turns;
    }
}
