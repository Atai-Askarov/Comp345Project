#include "Player.h"

#include <algorithm>
#include <memory>
#include <ostream>
#include <utility>

// Use standard library names directly, such as string, vector, and cout.
using namespace std;

// A default player starts with an empty command folder.
Player::Player() : Player("Unnamed player") {}

// Temporary smart pointers clean up automatically if an allocation fails.
// release() transfers responsibility for cleanup to Player's destructor.
Player::Player(const string& playerName)
    : name(nullptr), territories(nullptr), attackTargets(nullptr),
      hand(nullptr), orders(nullptr) {
    auto newName = make_unique<string>(playerName);
    auto newTerritories = make_unique<vector<Territory*>>();
    auto newTargets = make_unique<vector<Territory*>>();
    name = newName.release();
    territories = newTerritories.release();
    attackTargets = newTargets.release();
}

// Copy the folder's lists, not the map's Territory objects.
// Hand and OrdersList remain borrowed connections in this initial version.
// TODO: Revisit their ownership and copying when those classes are available.
Player::Player(const Player& other)
    : name(nullptr), territories(nullptr), attackTargets(nullptr),
      hand(nullptr), orders(nullptr) {
    // Keep ownership local until every allocation and copy succeeds.
    // If any copy throws, these smart pointers release all partial state.
    auto newName = make_unique<string>(*other.name);
    auto newTerritories = make_unique<vector<Territory*>>(*other.territories);
    auto newTargets = make_unique<vector<Territory*>>(*other.attackTargets);
    name = newName.release();
    territories = newTerritories.release();
    attackTargets = newTargets.release();
    hand = other.hand;
    orders = other.orders;
}

// Copy first, then swap: if copying fails, the original player stays intact.
Player& Player::operator=(const Player& other) {
    if (this != &other) {
        Player copy(other);
        swap(name, copy.name);
        swap(territories, copy.territories);
        swap(attackTargets, copy.attackTargets);
        swap(hand, copy.hand);
        swap(orders, copy.orders);
    }
    return *this;
}

// Delete only what Player allocated. Borrowed objects belong elsewhere.
Player::~Player() {
    delete name;
    delete territories;
    delete attackTargets;
}

// A const reference lets the caller read the name without copying/changing it.
const string& Player::getName() const { return *name; }

// Ignore missing territories and duplicates so the list stays meaningful.
void Player::addTerritory(Territory* territory) {
    if (territory && find(territories->begin(), territories->end(), territory)
                         == territories->end()) {
        territories->push_back(territory);
    }
}

// Expose the list for reading; callers cannot add/remove entries through it.
const vector<Territory*>& Player::getTerritories() const {
    return *territories;
}

// Return a separate list, like handing the general a photocopy of the folder.
vector<Territory*> Player::toDefend() const { return *territories; }

// Store unique, non-null targets supplied by the caller.
// This does not check adjacency or ownership; Map integration will do that.
void Player::setAttackTargets(const vector<Territory*>& targets) {
    vector<Territory*> filtered;
    for (Territory* target : targets) {
        if (target && find(filtered.begin(), filtered.end(), target)
                          == filtered.end()) {
            filtered.push_back(target);
        }
    }
    attackTargets->swap(filtered);
}

// Choosing targets does not execute an attack.
vector<Territory*> Player::toAttack() const { return *attackTargets; }

// Attach/read the player's borrowed card wallet.
void Player::setHand(Hand* playerHand) { hand = playerHand; }
Hand* Player::getHand() const { return hand; }

// Attach/read the player's borrowed order notebook.
void Player::setOrdersList(OrdersList* playerOrders) { orders = playerOrders; }
OrdersList* Player::getOrdersList() const { return orders; }

// Print a readable summary without needing teammates' class definitions.
ostream& operator<<(ostream& out, const Player& player) {
    out << "Player: " << *player.name
        << " | owned territories: " << player.territories->size()
        << " | attack targets: " << player.attackTargets->size()
        << " | hand: " << (player.hand ? "attached" : "not attached")
        << " | orders: " << (player.orders ? "attached" : "not attached");
    return out;
}
