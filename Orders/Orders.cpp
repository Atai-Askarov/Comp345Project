#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <random>
#include <limits>
#include <utility>
#include "Orders.h"
#include "../Player/Player.h"
#include "../Map/Map.h"

namespace {
// Turn state belongs here until the controller/Player provide a shared API.
std::vector<std::pair<Player*, Player*>> negotiations;
std::vector<Player*> conquerors;
std::unique_ptr<Player> neutral;

bool areNegotiating(Player* first, Player* second) {
    return std::any_of(negotiations.begin(), negotiations.end(), [=](const auto& pair) {
        return (pair.first == first && pair.second == second)
            || (pair.first == second && pair.second == first);
    });
}

Player* neutralPlayer() {
    if (!neutral) neutral = std::make_unique<Player>("Neutral");
    return neutral.get();
}
} // namespace

Orders::Orders() : effect(new std::string("Not executed")) {}
Orders::Orders(const Orders& other) : Subject(other), effect(new std::string(*other.effect)) {}
Orders& Orders::operator=(const Orders& other) {
    if (this != &other) {
        Subject::operator=(other);
        *effect = *other.effect;
    }
    return *this;
}
Orders::~Orders() { delete effect; }

void Orders::recordEffect(const std::string& text) {
    *effect = text;
    std::cout << getName() << ": " << text << '\n';
    notify(this);
}

const std::vector<Player*>& Orders::getConquerors() { return conquerors; }
void Orders::endTurn() { negotiations.clear(); conquerors.clear(); }
void Orders::resetGameState() {
    endTurn();
    if (neutral) {
        const auto territories = neutral->getTerritories();
        for (Territory* territory : territories) neutral->removeTerritory(territory);
        neutral.reset();
    }
}

//Orders stream

std::ostream& operator<<(std::ostream& os, const Orders& order) {
	return order.print(os);
}

std::string Orders::stringToLog() const {
	return "Order Executed: " + getName() + " | " + *effect;
}

//OrdersList

OrdersList::OrdersList() {}

//Destructor
OrdersList::~OrdersList() {
    for (Orders* order : oL) {
        delete order;
    }
}

//Copy Constructor
OrdersList::OrdersList(const OrdersList& other) {
    // Clean up partial clones if copying an order throws.
    try {
        for (Orders* order : other.oL) {
            std::unique_ptr<Orders> copy(order->clone());
            add(copy.get());
            copy.release();
        }
    } catch (...) {
        for (Orders* order : oL) {
            delete order;
        }
        throw;
    }
}

//Add method
void OrdersList::add(Orders* order) {
    if (!order) {
        throw std::invalid_argument("Cannot add a null order.");
    }
    if (std::find(oL.begin(), oL.end(), order) != oL.end()) {
        throw std::invalid_argument("Order already belongs to this list.");
    }
    oL.push_back(order);
    try { notify(this); }
    catch (...) { oL.pop_back(); throw; } // Caller retains ownership on failure.
}

std::string OrdersList::stringToLog() const {
    if (oL.empty()) {
        return "Order Issued: (none)";
    }
    return "Order Issued: " + oL.back()->getName();
}

//Remove method
void OrdersList::remove(int elemIndex) {
	size_t listLength = oL.size();
	if (listLength == 0) {
		throw std::runtime_error("Error: Can't remove from empty OrdersList.");
	}
	else if (elemIndex < 0 || static_cast<size_t>(elemIndex) >= listLength)
	{
		throw std::runtime_error("Error: Index out of bound.");
	}
	else
	{
		delete oL[elemIndex];
		oL.erase(oL.begin() + elemIndex);
	}
	
}

//Move method
void OrdersList::move(int iniIndex, int newIndex) {
    if (iniIndex < 0 || newIndex < 0
            || static_cast<size_t>(iniIndex) >= oL.size()
            || static_cast<size_t>(newIndex) >= oL.size()) {
        throw std::out_of_range("Order index out of bounds.");
    }
    // Reposition the selected order without swapping unrelated entries.
    Orders* order = oL[iniIndex];
    oL.erase(oL.begin() + iniIndex);
    oL.insert(oL.begin() + newIndex, order);
}

//Execute method
void OrdersList::execute() {
    if (oL.empty()) {
        throw std::runtime_error("No order to execute.");
    }
    while (!oL.empty()) {
        oL.front()->execute();
        remove(0); // Deletes only after execute() returns successfully.
    }
}

//Assignment operator overload
OrdersList& OrdersList::operator=(const OrdersList& copied) {
    if (this != &copied) {
        OrdersList copy(copied);
        oL.swap(copy.oL); // The temporary deletes the old orders.
    }
    return *this;
}

size_t OrdersList::getOrdersListSize() {
	return oL.size();
}

Orders* OrdersList::getOrder(int elemIndex) {
	
	if (elemIndex >= 0 && static_cast<size_t>(elemIndex) < oL.size())
	{
		return oL[elemIndex];
	}
	else
	{
		return nullptr;
	}
}

//Stream for whole list
std::ostream& operator<<(std::ostream& os, const OrdersList& copied) {
	unsigned listLength = copied.oL.size();
	for (unsigned i = 0; i < listLength; i++) {
		os << i + 1 << " --> " << copied.oL[i]->getName() << " || ";
	}

	os << "\n";
	
	return os;
}

std::vector<Orders*>* OrdersList::getOrdersList() {
	return &this->oL;
}
// Actions

Deploy::Deploy() : currentPlayer(nullptr), target(nullptr), amount(0), name("Deploy") {}

Deploy::Deploy(Player* player, Territory* terr, int qty) : currentPlayer(player), target(terr), amount(qty), name("Deploy") {}

std::string Deploy::getName() const {
	return name;
}
std::ostream& Deploy::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Deploy::validate() const {
    return currentPlayer && target && target->getOwner() == currentPlayer
        && amount > 0 && amount <= currentPlayer->getReinforcementPool()
        && target->getArmies() >= 0
        && amount <= std::numeric_limits<int>::max() - target->getArmies();
}

void Deploy::execute() {
    if (!validate()) { recordEffect("Invalid deploy; no effect"); return; }
    currentPlayer->spendReinforcements(amount);
    target->setArmies(target->getArmies() + amount);
    recordEffect("Deployed " + std::to_string(amount) + " armies to " + target->getName());
}

Orders* Deploy::clone() const {
	return new Deploy(*this);
}

Deploy::~Deploy() {
}

Advance::Advance() : currentPlayer(nullptr), source(nullptr), target(nullptr), amount(0), name("Advance") {}

Advance::Advance(Player* player, Territory* src, Territory* tgt, int qty) : currentPlayer(player), source(src), target(tgt), amount(qty), name("Advance") {}

std::string Advance::getName() const {
	return name;
}
std::ostream& Advance::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Advance::validate() const {
    if (!currentPlayer || !source || !target || source == target || amount <= 0
            || source->getOwner() != currentPlayer || source->getArmies() < amount
            || target->getArmies() < 0) return false;
    const auto& neighbours = source->getNeighbours();
    if (std::find(neighbours.begin(), neighbours.end(), target) == neighbours.end()) return false;
    if (target->getOwner() == currentPlayer)
        return amount <= std::numeric_limits<int>::max() - target->getArmies();
    return !areNegotiating(currentPlayer, target->getOwner());
}

void Advance::execute() {
    if (!validate()) { recordEffect("Invalid advance; no effect"); return; }
    if (target->getOwner() == currentPlayer) {
        source->setArmies(source->getArmies() - amount);
        target->setArmies(target->getArmies() + amount);
        recordEffect("Moved " + std::to_string(amount) + " armies from "
                     + source->getName() + " to " + target->getName());
    } else {
        attackSimulation(source, target, currentPlayer, amount);
        recordEffect(std::string(target->getOwner() == currentPlayer ? "Conquered " : "Attack repelled at ")
                     + target->getName() + "; source armies " + std::to_string(source->getArmies())
                     + ", target armies " + std::to_string(target->getArmies()));
    }
}

Orders* Advance::clone() const {
	return new Advance(*this);
}

void Advance::attackSimulation(Territory* simSource, Territory* simTarget, Player* simPlayer, int quantity) {
    // Preserve the existing public helper, but reject invalid direct calls too.
    Advance battle(simPlayer, simSource, simTarget, quantity);
    if (!battle.validate() || simTarget->getOwner() == simPlayer) return;
    static std::mt19937 random(std::random_device{}());
    std::bernoulli_distribution attackerHit(0.60), defenderHit(0.70);
    const int defenders = simTarget->getArmies();
    int attackerKills = 0, defenderKills = 0;
    // One simultaneous volley: defenders killed here still get their shots.
    for (int i = 0; i < quantity; ++i) if (attackerHit(random)) ++attackerKills;
    for (int i = 0; i < defenders; ++i) if (defenderHit(random)) ++defenderKills;
    const int survivingAttackers = std::max(0, quantity - defenderKills);
    const int survivingDefenders = std::max(0, defenders - attackerKills);
    if (survivingDefenders == 0) {
        // addTerritory updates both players' lists and the territory owner.
        // The handout makes capture conditional on eliminating all defenders.
        simPlayer->addTerritory(simTarget);
        simSource->setArmies(simSource->getArmies() - quantity);
        simTarget->setArmies(survivingAttackers);
        if (std::find(conquerors.begin(), conquerors.end(), simPlayer) == conquerors.end())
            conquerors.push_back(simPlayer);
    } else {
        // Surviving attackers retreat, preserving the teammate's original policy.
        simSource->setArmies(simSource->getArmies() - quantity + survivingAttackers);
        simTarget->setArmies(survivingDefenders);
    }
}

Advance::~Advance() {
}

Bomb::Bomb() : currentPlayer(nullptr), target(nullptr), name("Bomb") {}

Bomb::Bomb(Player* player, Territory* tgt) : currentPlayer(player), target(tgt), name("Bomb") {}

std::string Bomb::getName() const {
	return name;
}
std::ostream& Bomb::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Bomb::validate() const {
    if (!currentPlayer || !target || !target->getOwner()
            || target->getOwner() == currentPlayer || target->getArmies() < 0
            || areNegotiating(currentPlayer, target->getOwner())) return false;
    for (Territory* owned : currentPlayer->toDefend()) {
        const auto& neighbours = owned->getNeighbours();
        if (std::find(neighbours.begin(), neighbours.end(), target) != neighbours.end()) return true;
    }
    return false;
}

void Bomb::execute() {
    if (!validate()) { recordEffect("Invalid bomb; no effect"); return; }
    target->setArmies(target->getArmies() / 2);
    recordEffect("Bombed " + target->getName() + "; remaining armies " + std::to_string(target->getArmies()));
}

Orders* Bomb::clone() const {
	return new Bomb(*this);
}

Bomb::~Bomb() {
}

Blockade::Blockade() : currentPlayer(nullptr), target(nullptr), name("Blockade") {}

Blockade::Blockade(Player* player, Territory* tgt) : currentPlayer(player), target(tgt), name("Blockade") {}

std::string Blockade::getName() const {
	return name;
}
std::ostream& Blockade::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Blockade::validate() const {
    return currentPlayer && target && target->getOwner() == currentPlayer
        && target->getArmies() >= 0 && target->getArmies() <= std::numeric_limits<int>::max() / 2;
}

void Blockade::execute() {
    if (!validate()) { recordEffect("Invalid blockade; no effect"); return; }
    neutralPlayer()->addTerritory(target);
    target->setArmies(target->getArmies() * 2);
    recordEffect("Doubled armies on " + target->getName() + " and transferred ownership to Neutral");
}

Orders* Blockade::clone() const {
	return new Blockade(*this);
}

Blockade::~Blockade() {
}

Airlift::Airlift() : currentPlayer(nullptr), source(nullptr), target(nullptr), amount(0), name("Airlift") {}

Airlift::Airlift(Player* player, Territory* src, Territory* tgt, int qty) : currentPlayer(player), source(src), target(tgt), amount(qty), name("Airlift") {}

std::string Airlift::getName() const {
	return name;
}
std::ostream& Airlift::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Airlift::validate() const {
    return currentPlayer && source && target && source != target && amount > 0
        && source->getOwner() == currentPlayer && target->getOwner() == currentPlayer
        && source->getArmies() >= amount && target->getArmies() >= 0
        && amount <= std::numeric_limits<int>::max() - target->getArmies();
}

void Airlift::execute() {
    if (!validate()) { recordEffect("Invalid airlift; no effect"); return; }
    source->setArmies(source->getArmies() - amount);
    target->setArmies(target->getArmies() + amount);
    recordEffect("Airlifted " + std::to_string(amount) + " armies from "
                 + source->getName() + " to " + target->getName());
}

Orders* Airlift::clone() const {
	return new Airlift(*this);
}

Airlift::~Airlift() {
}

Negotiate::Negotiate() : currentPlayer(nullptr), targetPlayer(nullptr), name("Negotiate") {}

Negotiate::Negotiate(Player* player, Player* tPlayer) : currentPlayer(player), targetPlayer(tPlayer), name("Negotiate") {}

std::string Negotiate::getName() const {
	return name;
}
std::ostream& Negotiate::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Negotiate::validate() const {
    return currentPlayer && targetPlayer && currentPlayer != targetPlayer;
}

void Negotiate::execute() {
    if (!validate()) { recordEffect("Invalid negotiation; no effect"); return; }
    if (!areNegotiating(currentPlayer, targetPlayer)) negotiations.emplace_back(currentPlayer, targetPlayer);
    recordEffect("Attacks prevented between " + currentPlayer->getName() + " and "
                 + targetPlayer->getName());
}

Orders* Negotiate::clone() const {
	return new Negotiate(*this);
}

Negotiate::~Negotiate() {
}
