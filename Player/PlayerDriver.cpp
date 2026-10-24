#include "Player.h"
#include "../Cards/Cards.h"

#include <iostream>

using namespace std;

int main() {
    // Declared first so the borrowed Hand outlives all three Players.
    // Hand and Deck clean up their own cards when main ends.
    Hand linaHand;
    Deck deck;

    cout << "=== 1. Player 1 Creation ===\n";
    Player player1("Lina");
    player1.setHand(&linaHand);
    cout << "Lina has an empty Hand attached.\n";
    cout << "Player 1: " << player1 << "\n\n";

    cout << "=== 2. Player 2 Creation ===\n";
    Player player2("Alex");
    cout << "Player 2: " << player2 << "\n\n";

    cout << "=== 3. Copy Constructor ===\n";
    cout << "We will now create Player 3 by copying Player 1.\n";
    Player player3(player1);
    cout << "Player 1: " << player1 << '\n';
    cout << "Player 3: " << player3 << "\n\n";
    cout << "Player 3 borrows the same Hand as Player 1; the Hand is not copied.\n\n";

    cout << "=== 4. Assignment Operator ===\n";
    cout << "We will now assign Player 1's information to Player 2.\n";
    cout << "Before assignment:\n";
    cout << "Player 1: " << player1 << '\n';
    cout << "Player 2: " << player2 << '\n';
    player2 = player1;
    cout << "After assignment:\n";
    cout << "Player 1: " << player1 << '\n';
    cout << "Player 2: " << player2 << "\n\n";
    cout << "Player 2 now also borrows Player 1's Hand.\n\n";

    cout << "=== 5. Territories to Defend and Attack ===\n";
    cout << "Territories Lina would defend: " << player1.toDefend().size() << '\n';
    cout << "Territories Lina would attack: " << player1.toAttack().size() << '\n';
    cout << "Both lists are empty because no real territories are attached yet.\n";

    cout << "\n=== Player Hand Demonstration ===\n";
    cout << "Lina's Hand before adding cards:\n";
    cout << *player1.getHand();

    cout << "Adding a Bomb card to Lina's Hand.\n";
    linaHand.addCard(new Card(CardType::Bomb));
    cout << *player1.getHand();

    cout << "Adding an Airlift card to the deck, then drawing it into Lina's Hand.\n";
    deck.addCard(new Card(CardType::Airlift));
    deck.draw(*player1.getHand());
    cout << "Lina's updated Hand:\n" << *player1.getHand();
    cout << "Cards in Lina's Hand: " << player1.getHand()->getSize() << '\n';
    cout << "Cards remaining in the deck: " << deck.getSize() << "\n\n";

    cout << "Players 2 and 3 share this Hand, so they see the same cards:\n";
    cout << "Player 2's Hand:\n" << *player2.getHand();
    cout << "Player 3's Hand:\n" << *player3.getHand();
    cout << "Players borrow the Hand and do not delete it.\n\n";

    cout << "Real territories still require Map integration.\n";
    cout << "issueOrder() still requires the real Order and OrdersList interfaces.\n";

    return 0;
}
