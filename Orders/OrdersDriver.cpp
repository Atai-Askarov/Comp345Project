#include "Orders.h"

int main() {

	std::cout << "Testing Orders \n" << std::endl;
	// Create OrdersLists
	OrdersList player1;


	// OrdersList owns and deletes these dynamically allocated orders.
	// Do not add addresses of local objects to this owning list.
	player1.add(new Deploy());
	player1.add(new Advance());
	player1.add(new Bomb());
	player1.add(new Blockade());
	player1.add(new Airlift());
	player1.add(new Negotiate());
	player1.add(new Deploy());

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
