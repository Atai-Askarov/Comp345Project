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
	unsigned listLength = ol.oList.size();
	oList = std::vector<Orders*>(listLength);
	for (unsigned i = 0; i < listLength; i++) {
		oList[i] = ol.oList[i]->clone();
	}
}

//Add method
void OrdersList::add(Orders* order) {
	if(order) {
		oList.push_back(order);
	}
	else {
		std::runtime_error("Error: Adding null pointer to OrdersList.");
	}
}

//Remove method
void OrdersList::remove(int elemIndex) {
	unsigned listLength = oList.size();
	if (listLength == 0) {
		throw std::runtime_error("Error: Can't remove from empty OrdersList.");
	}
	else if (elemIndex < 0 || elemIndex >= listLength)
	{
		throw std::runtime_error("Error: Index out of bound.")
	}
	else
	{
		oList.erase(oList.begin() + elemIndex);
	}
	
}

//Move method
void:OrdersList::move(int iniIndex, int newIndex) {
	unsigned listLength = oList.size();
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
		Orders* temp = oList[iniIndex];
		oList[iniIndex] = oList[newIndex];
		oList[newIndex] = temp;
	}
}

//Execute method
void:OrdersList::execute() {
	unsigned listLength = oList.size();
	if (listLength == 0) {
		throw std::runtime_error("Error: No order to execute.");
	}
	else {
		for (auto it = oList.begin(); it != oList.end();) {
			if (!oList.empty()) {
				it->execute();
				it = oList.erase(it);
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

	unsigned copiedListLength = copied.oList.size();

	for (auto o = oList.begin(); o != oList.end();) {
		{
			o = oList.erase(o);
		}
	}

	oList = std::vector<Orders*>(copiedListLength);

	for (int oo = 0; oo < copiedListLength; oo++) {
		oList[oo] = copied.oList[oo]->clone();
	}
	return *this;
}

size_t OrdersList::getOrdersListSize() {
	return oList.size();
}

Orders* OrdersList::getOrder(int elemIndex) {
	
	if (elemIndex >= 0 && elemIndex < oList.size())
	{
		return oList[elemIndex];
	}
	else
	{
		return nullptr;
	}
}

//Stream for whole list
std::ostream& operator<<(std::ostream& os, const OrdersList& copied) {
	unsigned listLength = copied.oList.size();
	for (unsigned i = 0; i < listLength; i++) {
		os << i + 1 << " --> " << copied.oList[i]->getName() << " || ";
	}

	os << "\n";
	
	return os;
}

std::vector<Orders*>* OrdersList::getOrdersList() {
	return &this->oList;
}

// Actions

Deploy::Deploy() : {}

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

Advance::Advance() : {}

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

Bomb::Bomb() : {}

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

Blockade::Blockade() : {}

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

Airlift::Airlift() : {}

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

Negotiate::Negotiate() : {}

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
