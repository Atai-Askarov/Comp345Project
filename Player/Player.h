#ifndef PLAYER_H
#define PLAYER_H

#include <iosfwd>
#include <string>
#include <vector>

// Use standard library names directly, such as string, vector, and cout.
using namespace std;

// Forward declarations are like reserving names for our teammates' classes.
// We can store their addresses without needing their implementations yet.
class Territory;
class Hand;
class OrdersList;

// A Player is the general's command folder: territories, cards, and orders.
class Player {
public:
// “All classes must implement a correct copy constructor, assignment operator, and stream insertion operator.”
    Player();
    Player(const string& name);
    Player(const Player& other);
    Player& operator=(const Player& other);
    ~Player();

    const string& getName() const;
    void addTerritory(Territory* territory);
    const vector<Territory*>& getTerritories() const;

    // For Assignment 1, defending all owned territories is a simple strategy.
    vector<Territory*> toDefend() const;

    // Until Map provides adjacency information, targets are supplied explicitly.
    void setAttackTargets(const vector<Territory*>& targets);


    vector<Territory*> toAttack() const;

    // These connections borrow objects: Player does not delete them.
    void setHand(Hand* hand);
    Hand* getHand() const;
    void setOrdersList(OrdersList* orders);
    OrdersList* getOrdersList() const;

    // TODO: Add issueOrder() once Orders exposes order creation and insertion.
    // It must create an Order and add it to this player's OrdersList.

    friend ostream& operator<<(ostream& out, const Player& player);

private:
    // Player owns these containers, but borrows the Territory objects inside.
    // Pointer members also satisfy the assignment's class-member requirement.
    string* name;
    // Owned list of borrowed territory pointers.
    vector<Territory*>* territories;
    vector<Territory*>* attackTargets;

    // Temporary borrowed connections until Hand/OrdersList are implemented.
    // The caller must keep these objects alive while Player uses them.
    Hand* hand;
    OrdersList* orders;
};

#endif
