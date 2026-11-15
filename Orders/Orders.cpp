#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include "Orders.h"


Orders::~Orders() {}

//Orders stream

std::ostream& operator<<(std::ostream& os, const Orders& order) {
	return order.print(os);
}

std::string Orders::stringToLog() const {
	return "Order Executed: " + getName();
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
    notify(this);
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

Deploy::Deploy(Player* player, Territory* terr, int qty) : currentPlayer(player), target(terr), amount(qty), name("Deploy") {}

std::string Deploy::getName() const {
	return name;
}
std::ostream& Deploy::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Deploy::validate() const {
	std::cout << "Validating Deploy order..." << std::endl;
	std::cout << "TO BE IMPLEMENTED" << std::endl;
	if (currentPlayer != target->getOwner()) {
		std::cout << "You do not possess the territory you are trying to deploy to." << std::endl;
		return false;
	}
	// else if (amount > ) {}             ---- no reinforcementPool variable?
	else {
		std::cout << "Deploy order validated successfully." << std::endl;
		return true; // No invalid inputs

	}
}

void Deploy::execute() {
	if (validate()) {
		std::cout << "Executing Deploy order..." << std::endl;
		target->setArmies(target->getArmies() + amount);
		std::cout << "Deploy order executed successfully." << std::endl;
		
		notify(this);
	}
}

Orders* Deploy::clone() const {
	return new Deploy(*this);
}

Deploy::~Deploy() {
}

Advance::Advance(Player* player, Territory* src, Territory* tgt, int qty) : currentPlayer(player), source(src), target(tgt), amount(qty), name("Advance") {}

std::string Advance::getName() const {
	return name;
}
std::ostream& Advance::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Advance::validate() const {
	std::cout << "Validating Advance order..." << std::endl;
	if (source->getOwner() != currentPlayer) {
		std::cout << "You do not own the source territory." << std::endl;
		return false;
	}
	else if (std::find(source->getNeighbours()->begin(), source->getNeighbours()->end(), target) == source->getNeighbours()->end()) {
		std::cout << "The target territory is not adjacent to the source territory." << std::endl;
		return false;
	}
	else {
		std::cout << "Advance order validated successfully." << std::endl;
	return true; // No invalid inputs
	}
}

void Advance::execute() {
	if (validate()) {
		std::cout << "Executing Advance order..." << std::endl;
		if (source->getOwner() == target->getOwner()) { // Simply moving armies across owned territories
			source->setArmies(source->getArmies() - amount);
			target->setArmies(target->getArmies() + amount);
		}
		else { // Moving on enemy territory; an attack
			if (false) { // Placeholder for negotiation check
				std::cout << "You can't target this player for an attack." << std::endl;
				return;
			}
			attackSimulation(source, target, currentPlayer, amount);
		}
		std::cout << "Advance order executed successfully." << std::endl;
		
		
		notify(this);
	}
}

Orders* Advance::clone() const {
	return new Advance(*this);
}

void Advance::attackSimulation(Territory* simSource, Territory* simTarget, Player* simCurrentPlayer, int simQty) {
	int atkKills = 0;
	int defKills = 0;
	int aliveAttackers = 0;
	int aliveAttackees = 0;

	simSource->setArmies(simSource->getArmies() - simQty);

	for (int i = 1; i <= army; i++) // Attackers sim
	{

		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> range(0, 100);

		int roll = (int)range(rng) % 100 + 1;
		if (roll <= 60)
		{
			atkKills++;
		}
	}

	for (int i = 1; i <= simTarget->getArmies(); i++) // Attackee sim
	{

		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> range(0, 100);

		int roll = (int)range(rng) % 100 + 1;

		if (roll <= 70)
		{
			defKills++;
		}
	}

	aliveAttackers = max(0, simQty - defKills);
	aliveAttackees = max(0, simTarget->getArmies() - atkKills);

	if (aliveAttackees == 0) { //Attack take territory
		std::cout << simCurrentPlayer->getName() << " has successfully conquered " << simTarget->getName() << "!" << std::endl;
		if (simTarget->getOwner() != nullptr){
			simTarget->getOwner()->removeTerritory(*simTarget);
			
		}
		simTarget->setOwner(simCurrentPlayer);
		simCurrentPlayer->addTerritory(*simTarget);
		simTarget->setArmies(aliveAttackers);
		
		//Missing card for 1 conquered territory
	}
	else {
		std::cout << simCurrentPlayer->getName() << " has failed to conquer " << simTarget->getName() << "!" << std::endl;
		pSource->setArmies(pSource->getArmies() + remainingAttackArmies); // Attackers retreat
		pTarget->setArmies(remainingDefendArmies);
		simSource->setArmies(simSource->getArmies() + aliveAttackers); // Attackers retreat
		simTarget->setArmies(aliveAttackees);
	})

}
//REMAKE territory ownership switch. addterritory method already handles it.


Advance::~Advance() {
}

Bomb::Bomb(Player* player, Territory* tgt) : currentPlayer(player), target(tgt), name("Bomb") {}

std::string Bomb::getName() const {
	return name;
}
std::ostream& Bomb::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Bomb::validate() const {
	std::cout << "Validating Bomb order..." << std::endl;
	if (target->getOwner() == currentPlayer) {
		std::cout << "You can't bomb your own territory." << std::endl;
		return false;
	}
	for (auto const& terrVector : currentPlayer->getTerritories()) {
		auto it = std::find(terrVecor->getNeighbours()->begin(), terrVector->getNeighbours()->end(), target->getName());
		if (it != terrVector->getNeighbours()->end()) {
			std::cout << "Bomb order validated successfully." << std::endl;
			return true; // Valid if any of the player's territories are adjacent to the target
		}
	}
	std::cout << "The target territory is not adjacent to any of your territories." << std::endl;
	return false; 
}

void Bomb::execute() {
	if (validate()) {
		std::cout << "Executing Bomb order..." << std::endl;
		target->setArmies(target->getArmies() / 2);
		std::cout << "Bomb order executed successfully." << std::endl;
		
		
		notify(this);
	}
}

Orders* Bomb::clone() const {
	return new Bomb(*this);
}

Bomb::~Bomb() {
}

Blockade::Blockade(Player* player, Territory* tgt) : currentPlayer(player), target(tgt), name("Blockade") {}

std::string Blockade::getName() const {
	return name;
}
std::ostream& Blockade::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Blockade::validate() const {
	std::cout << "Validating Blockade order..." << std::endl;
	if (target->getOwner() != currentPlayer) {
		std::cout << "You can't blockade a territory you don't own." << std::endl;
		return false;
	}
	else {
		std::cout << "Blockade order validated successfully." << std::endl;
		return true; 
	}
}

void Blockade::execute() {
	if (validate()) {
		std::cout << "Executing Blockade order..." << std::endl;
		target->setArmies(target->getArmies() * 2);
		target->getOwner()->removeTerritory(*target);
		target->setOwner(nullptr); // Territory becomes neutral
		std::cout << "Blockade order executed successfully." << std::endl;
		
		
		notify(this);
	}
}

Orders* Blockade::clone() const {
	return new Blockade(*this);
}

Blockade::~Blockade() {
}

Airlift::Airlift(Player* player, Territory* src, Territory* tgt, int qty) : currentPlayer(player), source(src), target(tgt), amount(qty) name("Airlift") {}

std::string Airlift::getName() const {
	return name;
}
std::ostream& Airlift::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Airlift::validate() const {
	std::cout << "Validating Airlift order..." << std::endl;
	if (source->getOwner() != currentPlayer && target->getOwner() != currentPlayer) {
		std::cout << "You can't airlift from or to territories that aren't owned by you." << std::endl;
		return false;
	}
	else if (source == target) {
		std::cout << "You can't airlift to the same territory." << std::endl;
		return false;
	}
	else if (source->getArmies() < amount) {
		std::cout << "You don't have this many armies to airlift." << std::endl;
		return false;
	}
	else if (amount <= 0) {
		std::cout << "You can't airlift a non-positive number of armies." << std::endl;
		return false;
	}
	else {
		std::cout << "Airlift order validated successfully." << std::endl;
		return true; // No invalid inputs
	}
}

void Airlift::execute() {
	if (validate()) {
		std::cout << "Executing Airlift order..." << std::endl;
		source->setArmies(source->getArmies() - amount);
		target->setArmies(target->getArmies() + amount);
		
		std::cout << "Airlift order executed successfully." << std::endl;
		
		notify(this);
	}
}

Orders* Airlift::clone() const {
	return new Airlift(*this);
}

Airlift::~Airlift() {
}

Negotiate::Negotiate(Player* player, Player* tPlayer) : currentPlayer(player), targetPlayer(tPlayer), name("Negotiate") {}

std::string Negotiate::getName() const {
	return name;
}
std::ostream& Negotiate::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Negotiate::validate() const {
	std::cout << "Validating Negotiate order..." << std::endl;
	if (targetPlayer == currentPlayer) {
		std::cout << "You can't negotiate with yourself." << std::endl;
		return false;

	}
	else {
		std::cout << "Negotiate order validated successfully." << std::endl;
		return true; // Placeholder
	}
}

void Negotiate::execute() {
	if (validate()) {
		std::cout << "Executing Negotiate order..." << std::endl;

		// NEED A PLAYER ATTRIBUTE TO STORE NEGOTIATION STATUS

		std::cout << "Negotiate order executed successfully." << std::endl;
		
		
		notify(this);
	}
}

Orders* Negotiate::clone() const {
	return new Negotiate(*this);
}

Negotiate::~Negotiate() {
}
