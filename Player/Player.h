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
    // Copies references without transferring ownership. Transfer/remove any
    // territories owned by the destination before replacing its reference list.
    Player& operator=(const Player& other);
    ~Player();

    const string& getName() const;
    void addTerritory(Territory* territory);
    void removeTerritory(Territory* territory);
    // Stored references may include another player's territories after copying.
    const vector<Territory*>& getTerritories() const;

    // For Assignment 1, defending all owned territories is a simple strategy.
    vector<Territory*> toDefend() const;

    // Legacy manual list, retained for compatibility. toAttack() now discovers
    // targets from Map neighbours rather than using this stored list.
    void setAttackTargets(const vector<Territory*>& targets);


    vector<Territory*> toAttack() const;

    // These connections borrow objects: Player does not delete them.
    void setHand(Hand* hand);
    Hand* getHand() const;
    void setOrdersList(OrdersList* orders);
    OrdersList* getOrdersList() const;

    // Create and store an order without executing it. Requires an attached list.
    // Types: deploy, advance, bomb, blockade, airlift, negotiate (lowercase).
    void issueOrder(const string& orderType = "deploy");

    friend ostream& operator<<(ostream& out, const Player& player);

private:
    // Player owns these containers, but borrows the Territory objects inside.
    // Pointer members also satisfy the assignment's class-member requirement.
    string* name;
    // Owned list of borrowed territory pointers.
    vector<Territory*>* territories;
    vector<Territory*>* attackTargets;

    // Borrowed connections, shared by copies rather than cloned.
    // The caller must keep these objects alive while Player uses them.
    Hand* hand;
    OrdersList* orders;
};

#endif
