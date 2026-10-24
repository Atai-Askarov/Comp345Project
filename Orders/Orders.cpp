#include <cstdlib>
#include <iomanip>
#include "Orders.h"


//Orders stream

std::ostream& operator<<(std::ostream& os, const Orders& order) {
	return order.print(os);
}

//OrdersList

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
