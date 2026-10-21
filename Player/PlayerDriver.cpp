#include "Player.h"

#include <iostream>

using namespace std;

int main() {
    cout << "=== 1. Player 1 Creation ===\n";
    Player player1("Lina");
    cout << "Player 1: " << player1 << "\n\n";

    cout << "=== 2. Player 2 Creation ===\n";
    Player player2("Alex");
    cout << "Player 2: " << player2 << "\n\n";

    cout << "=== 3. Copy Constructor ===\n";
    cout << "We will now create Player 3 by copying Player 1.\n";
    Player player3(player1);
    cout << "Player 1: " << player1 << '\n';
    cout << "Player 3: " << player3 << "\n\n";

    cout << "=== 4. Assignment Operator ===\n";
    cout << "We will now assign Player 1's information to Player 2.\n";
    cout << "Before assignment:\n";
    cout << "Player 1: " << player1 << '\n';
    cout << "Player 2: " << player2 << '\n';
    player2 = player1;
    cout << "After assignment:\n";
    cout << "Player 1: " << player1 << '\n';
    cout << "Player 2: " << player2 << "\n\n";

    cout << "=== 5. Territories to Defend and Attack ===\n";
    cout << "Territories Lina would defend: " << player1.toDefend().size() << '\n';
    cout << "Territories Lina would attack: " << player1.toAttack().size() << '\n';
    cout << "Both lists are empty because no real territories are attached yet.\n";
    cout << "Cards and orders will be demonstrated after teammate integration.\n";

    return 0;
}
