#ifndef ORDERSLIST_H
#define ORDERSLIST_H

#include "Orders.h"

class OrdersList {
public:
	OrdersList();
	OrdersList(const string& playerName);
	OrdersList(const OrdersList& copied);
	OrdersList& operator=(const OrdersList& copied);
	~OrdersList();


	//OrdersList(const string& type, const vector<Orders>& orders);
	
	void move(const string& order);
	const vector<Orders*> remove(const string& order);
	