#ifndef ORDERS_H
#define ORDERS_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using namespace	std;


class Orders {
public:
	virtual ~Orders();

	//Order's type
	
	virtual std::string getName() const = 0;

	//Verify it's a real order

	virtual bool validate() const = 0;

	//Execute order

	virtual void execute() = 0;

	//Clone Order

	virtual Orders* clone() const = 0;


private:
	//Overloading

	virtual std::ostream& print(std::ostream&) const = 0;

	//Stream insertion operator

	friend std::ostream& operator<<(std::ostream&, const Orders&);

};

class OrdersList {

private:
	// Owns each stored order; callers must not delete an inserted order.
	std::vector<Orders*> oL{};

public:
	//Constructor & Destructor

	OrdersList();
	OrdersList(const OrdersList& ol);
	//OrdersList(const string& user, const vector<Orders>& ol); -- might be useful later
	~OrdersList();

	//Operator Overloading

	OrdersList& operator=(const OrdersList& copied);


	//Acessor & Mutators

	// Takes ownership after successful insertion.
	void add(Orders* order);
	void remove(int elemIndex);
	void move(int iniIndex, int newIndex);

	//const vector<Orders*> remove(const string& order); -- removing per order type could be useful

	//Iterate through the list
	void execute();

	//Getters

	std::vector<Orders*>* getOrdersList();

	Orders* getOrder(int elemIndex);

	size_t getOrdersListSize();

private:
	friend std::ostream& operator<<(std::ostream&, const OrdersList&);
};

class Deploy : public Orders {
public:
	Deploy();
	~Deploy() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Advance : public Orders {
public:
	Advance();
	~Advance() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Bomb : public Orders {
public:
	Bomb();
	~Bomb() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Blockade : public Orders {
public:
	Blockade();
	~Blockade() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Airlift : public Orders {
public:
	Airlift();
	~Airlift() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Negotiate : public Orders {
public:
	Negotiate();
	~Negotiate() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

#endif


