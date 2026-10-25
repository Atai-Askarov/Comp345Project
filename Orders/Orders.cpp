#include <cstdlib>
#include <iomanip>
#include "Orders.h"


//Orders stream

std::ostream& operator<<(std::ostream& os, const Orders& order) {
	return order.print(os);
}

//OrdersList

OrdersList::OrdersList() {}

//Destructor
OrdersList::~OrdersList{
	//vector<Orders*>().swap(ol);
}

//Copy Constructor
OrdersList::OrdersList(const OrdersList& ol) {
	unsigned listLength = ol.oL.size();
	oL = std::vector<Orders*>(listLength);
	for (unsigned i = 0; i < listLength; i++) {
		oL[i] = ol.oL[i]->clone();
	}
}

//Add method
void OrdersList::add(Orders* order) {
	if(order) {
		oL.push_back(order);
	}
	else {
		std::runtime_error("Error: Adding null pointer to OrdersList.");
	}
}

//Remove method
void OrdersList::remove(int elemIndex) {
	unsigned listLength = oL.size();
	if (listLength == 0) {
		throw std::runtime_error("Error: Can't remove from empty OrdersList.");
	}
	else if (elemIndex < 0 || elemIndex >= listLength)
	{
		throw std::runtime_error("Error: Index out of bound.");
	}
	else
	{
		oL.erase(oL.begin() + elemIndex);
	}
	
}

//Move method
void OrdersList::move(int iniIndex, int newIndex) {
	unsigned listLength = oL.size();
	if (listLength == 0) {
		throw std::runtime_error("Error: No values to move.");
	}
	else if (listLength == 1)
	{
		throw std::runtime_error("Error: The singular value can't move.");
	}
	else if (iniIndex < 0 || iniIndex >= listLength || newIndex < 0 || newIndex >= listLength)
	{
		throw std::runtime_error("Error: At least one index is out of bounds.");
	}
	else
	{
		Orders* temp = oL[iniIndex];
		oL[iniIndex] = oL[newIndex];
		oL[newIndex] = temp;
	}
}

//Execute method
void OrdersList::execute() {
	unsigned listLength = oL.size();
	if (listLength == 0) {
		throw std::runtime_error("Error: No order to execute.");
	}
	else {
		for (auto it = oL.begin(); it != oL.end();) {
			if (!oL.empty()) {
				it->execute();
				it = oL.erase(it);
			}
			else {
				++it;
			}
		}
	}
	
}

//Assignment operator overload
OrdersList& OrdersList::operator=(const OrdersList& copied) {
	//Assigning left to right side
	if (&copied == this) {
		return *this;
	}

	unsigned copiedListLength = copied.oL.size();

	for (auto o = oL.begin(); o != oL.end();) {
		{
			o = oL.erase(o);
		}
	}

	oL = std::vector<Orders*>(copiedListLength);

	for (size_t oo = 0; oo < copiedListLength; oo++) {
		oL[oo] = copied.oL[oo]->clone();
	}
	return *this;
}

size_t OrdersList::getOrdersListSize() {
	return oL.size();
}

Orders* OrdersList::getOrder(int elemIndex) {
	
	if (elemIndex >= 0 && elemIndex < oL.size())
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

Deploy::Deploy() { Deploy::name = "Deploy"; }

const std::string Deploy::name = "Deploy";
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

Advance::Advance(){}

const std::string Advance::name = "Advance";
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

Bomb::Bomb(){}

const std::string Bomb::name = "Bomb";
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

Blockade::Blockade(){}

const std::string Blockade::name = "Blockade";
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

Airlift::Airlift(){}

const std::string Airlift::name = "Airlift";
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

Negotiate::Negotiate(){}

const std::string Negotiate::name = "Negotiate";
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
