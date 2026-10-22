#ifndef ORDERS_H
#define ORDERS_H

#include <iosfwd>
#include <string>
#include <vector>


using namespace	std;


class Orders {
public:
	Orders();
	Orders(const string& type);
	Orders(const Orders& copied);
	Orders& operator=(const Orders& copied);
	~Orders();

	bool validate(const Orders& order);

	void execute();

};

#endif // 

