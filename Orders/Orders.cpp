#include <cstdlib>
#include <iomanip>
#include "Orders.h"


//Orders stream

std::ostream& operator<<(std::ostream& os, const Orders& order) {
	return order.print(os);
}

//OrdersList
