#include "Orders.h"

int main() {

	std::cout << "Testing Orders \n" << std::endl;
	// Create OrdersLists
	OrdersList player1;


	// Create some orders
	Deploy aDeploy;
	Advance anAdvance;
	Bomb aBomb;
	Blockade aBlockade;
	Airlift anAirlift;
	Negotiate aNegotiate;
	Deploy anotherDeploy;
	
	// Add orders to the list
	player1.add(&aDeploy);
	player1.add(&anAdvance);
	player1.add(&aBomb);
	player1.add(&aBlockade);
	player1.add(&anAirlift);
	player1.add(&aNegotiate);
	player1.add(&anotherDeploy);

	// Print the orders list
	std::cout << "Orders List: " << std::endl;
	std::cout << player1;

	//Remove an order from the list
	player1.remove(6);

	// Print the orders list
	std::cout << "Orders List: " << std::endl;
	std::cout << player1;

	//Move an order in the list
	player1.move(1, 2);

	// Print the orders list
	std::cout << "Orders List: " << std::endl;
	std::cout << player1;

	// Execute each order
	player1.execute();
	
}