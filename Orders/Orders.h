#ifndef ORDERS_H
#define ORDERS_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../Logger/LoggingObserver.h"

using namespace	std;

class Player; // Forward declaration to avoid circular dependency
class Territory; // Forward declaration to avoid circular dependency

class Orders : public Subject, public ILoggable {
public:
	Orders();
	Orders(const Orders& other);
	Orders& operator=(const Orders& other);
	virtual ~Orders();

	//Order's type
	
	virtual std::string getName() const = 0;

	//Verify it's a real order

	virtual bool validate() const = 0;

	//Execute order

	virtual void execute() = 0;

	//Clone Order

	virtual Orders* clone() const = 0;

	//Log line written when the order is executed

	std::string stringToLog() const override;

	// The game controller awards one card to each listed player before endTurn().
	// These borrowed pointers stay valid only while the participating Players live.
	static const std::vector<Player*>& getConquerors();
	static void endTurn(); // Expire negotiations and clear this turn's conquests.
	// Call with no pending orders, BEFORE the old map/players are destroyed.
	// Releases Neutral ownership while its borrowed territories are still alive;
	// the module supports one running game at a time.
	static void resetGameState();

protected:
	// Store the effect before notifying the existing logging observers.
	void recordEffect(const std::string& text);


private:
	std::string* effect; // Owned; copies have independent effect strings.
	//Overloading

	virtual std::ostream& print(std::ostream&) const = 0;

	//Stream insertion operator

	friend std::ostream& operator<<(std::ostream&, const Orders&);

};

class OrdersList : public Subject, public ILoggable {

private:
	// Owns each stored order; callers must not delete an inserted order.
	std::vector<Orders*> oL{};

public:
	//Constructor & Destructor

	OrdersList();
	OrdersList(const OrdersList& ol);
	~OrdersList();

	//Operator Overloading

	OrdersList& operator=(const OrdersList& copied);


	//Acessor & Mutators

	// Takes ownership after successful insertion.
	void add(Orders* order);
	void remove(int elemIndex);
	void move(int iniIndex, int newIndex);


	//Iterate through the list
	void execute();

	//Getters

	std::vector<Orders*>* getOrdersList();

	Orders* getOrder(int elemIndex);

	size_t getOrdersListSize();

	//Log line written when an order is added

	std::string stringToLog() const override;

private:
	friend std::ostream& operator<<(std::ostream&, const OrdersList&);
};

class Deploy : public Orders {
public:
	Deploy(Player* player, Territory* terr, int qty);
	~Deploy() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	Player* currentPlayer;
	Territory* target;
	int amount;
	std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Advance : public Orders {
public:
	Advance(Player* player, Territory* src, Territory* tgt, int qty);
	~Advance() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;
	void attackSimulation(Territory*, Territory*, Player*, int);

private:
	Player* currentPlayer;
	Territory* source;
	Territory* target;
	int amount;
	std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Bomb : public Orders {
public:
	Bomb(Player* player, Territory* tgt);
	~Bomb() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	Player* currentPlayer;
	Territory* target;
	std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Blockade : public Orders {
public:
	Blockade(Player* player, Territory* tgt);
	~Blockade() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	Player* currentPlayer;
	Territory* target;
	std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Airlift : public Orders {
public:
	Airlift(Player* player, Territory* src, Territory* tgt, int qty);
	~Airlift() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	Player* currentPlayer;
	Territory* source;
	Territory* target;
	int amount;
	std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

class Negotiate : public Orders {
public:
	Negotiate(Player* player, Player* tPlayer);
	~Negotiate() override;
	std::string getName() const override;
	bool validate() const override;
	void execute() override;

private:
	Player* currentPlayer;
	Player* targetPlayer;
	std::string name;
	Orders* clone() const override;
	std::ostream& print(std::ostream&) const override;
};

#endif


