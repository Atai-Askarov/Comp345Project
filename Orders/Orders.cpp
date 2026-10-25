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

Deploy::Deploy() : name("Deploy") {}

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
	return true; // Placeholder
}

void Deploy::execute() {
	if (validate()) {
		std::cout << "Executing Deploy order..." << std::endl;
		std::cout << "TO BE IMPLEMENTED" << std::endl;
		// Placeholder
	}
}

Orders* Deploy::clone() const {
	return new Deploy(*this);
}

Deploy::~Deploy() {
}

Advance::Advance() : name("Advance") {}

std::string Advance::getName() const {
	return name;
}
std::ostream& Advance::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Advance::validate() const {
	std::cout << "Validating Advance order..." << std::endl;
	std::cout << "TO BE IMPLEMENTED" << std::endl;
	return true; // Placeholder
}

void Advance::execute() {
	if (validate()) {
		std::cout << "Executing Advance order..." << std::endl;
		std::cout << "TO BE IMPLEMENTED" << std::endl;
		// Placeholder
	}
}

Orders* Advance::clone() const {
	return new Advance(*this);
}

Advance::~Advance() {
}

Bomb::Bomb() : name("Bomb") {}

std::string Bomb::getName() const {
	return name;
}
std::ostream& Bomb::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Bomb::validate() const {
	std::cout << "Validating Bomb order..." << std::endl;
	std::cout << "TO BE IMPLEMENTED" << std::endl;
	return true; // Placeholder
}

void Bomb::execute() {
	if (validate()) {
		std::cout << "Executing Bomb order..." << std::endl;
		std::cout << "TO BE IMPLEMENTED" << std::endl;
		// Placeholder
	}
}

Orders* Bomb::clone() const {
	return new Bomb(*this);
}

Bomb::~Bomb() {
}

Blockade::Blockade() : name("Blockade") {}

std::string Blockade::getName() const {
	return name;
}
std::ostream& Blockade::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Blockade::validate() const {
	std::cout << "Validating Blockade order..." << std::endl;
	std::cout << "TO BE IMPLEMENTED" << std::endl;
	return true; // Placeholder
}

void Blockade::execute() {
	if (validate()) {
		std::cout << "Executing Blockade order..." << std::endl;
		std::cout << "TO BE IMPLEMENTED" << std::endl;
		// Placeholder
	}
}

Orders* Blockade::clone() const {
	return new Blockade(*this);
}

Blockade::~Blockade() {
}

Airlift::Airlift() : name("Airlift") {}

std::string Airlift::getName() const {
	return name;
}
std::ostream& Airlift::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Airlift::validate() const {
	std::cout << "Validating Airlift order..." << std::endl;
	std::cout << "TO BE IMPLEMENTED" << std::endl;
	return true; // Placeholder
}

void Airlift::execute() {
	if (validate()) {
		std::cout << "Executing Airlift order..." << std::endl;
		std::cout << "TO BE IMPLEMENTED" << std::endl;
		// Placeholder
	}
}

Orders* Airlift::clone() const {
	return new Airlift(*this);
}

Airlift::~Airlift() {
}

Negotiate::Negotiate() : name("Negotiate") {}

std::string Negotiate::getName() const {
	return name;
}
std::ostream& Negotiate::print(std::ostream& os) const {
	os << "Order: " << name;
	return os;
}

bool Negotiate::validate() const {
	std::cout << "Validating Negotiate order..." << std::endl;
	std::cout << "TO BE IMPLEMENTED" << std::endl;
	return true; // Placeholder
}

void Negotiate::execute() {
	if (validate()) {
		std::cout << "Executing Negotiate order..." << std::endl;
		std::cout << "TO BE IMPLEMENTED" << std::endl;
		// Placeholder
	}
}

Orders* Negotiate::clone() const {
	return new Negotiate(*this);
}

Negotiate::~Negotiate() {
}
