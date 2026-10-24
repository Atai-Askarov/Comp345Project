#ifndef ORDERS_H
#define ORDERS_H

#include <iosfwd>
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
	std::vector<Orders*> oL{};

public:
	//Constructor & Destructor

	OrdersList(const OrdersList& ol);
	//OrdersList(const string& user, const vector<Orders>& ol); -- might be useful later
	~OrdersList();

	//Operator Overloading

	OrdersList& operator=(const OrdersList& copied);


	//Acessor & Mutators

	void add(Orders* order);
	void remove(int elemIndex);
	void move(int iniIndex, int newIndex);

	//const vector<Orders*> remove(const string& order); -- removing per order type could be useful

	//Getters

	std::vector<Orders*>* getOrdersList();

	Orders* getOrder(int elemIndex);

	size_t getOrdersListSize();

private:
	friend std::ostream& operator<<(std::ostream&, const OrdersList&);
};

class deploy : public Orders {
public:
	deploy();
	~deploy() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class advance : public Orders {
public:
	advance();
	~advance() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class bomb : public Orders {
public:
	bomb();
	~bomb() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class blockade : public Orders {
public:
	blockade();
	~blockade() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class airlift : public Orders {
public:
	airlift();
	~airlift() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class negotiate : public Orders {
public:
	negotiate();
	~negotiate() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	const std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

#endif


