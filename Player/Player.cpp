#include "Player.h"
#include "../Map/Map.h"

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
// All borrowed pointers remain shared; copying never changes territory owners.
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

// Register a territory once, then transfer it from its previous owner.
void Player::addTerritory(Territory* territory) {
    if (!territory) {
        return;
    }
    // Allocate before changing ownership, so allocation failure leaves it intact.
    if (find(territories->begin(), territories->end(), territory)
            == territories->end()) {
        territories->push_back(territory);
    }
    Player* previousOwner = territory->getOwner();
    if (previousOwner && previousOwner != this) {
        previousOwner->removeTerritory(territory);
    }
    territory->setOwner(this);
}

// Removing a copied reference must not clear the real owner's connection.
void Player::removeTerritory(Territory* territory) {
    territories->erase(remove(territories->begin(), territories->end(), territory),
                       territories->end());
    if (territory && territory->getOwner() == this) {
        territory->setOwner(nullptr);
    }
}

// Expose the list for reading; callers cannot add/remove entries through it.
const vector<Territory*>& Player::getTerritories() const {
    return *territories;
}

// Copies can contain shared references without owning those territories.
vector<Territory*> Player::toDefend() const {
    vector<Territory*> owned;
    for (Territory* territory : *territories) {
        if (territory && territory->getOwner() == this) {
            owned.push_back(territory);
        }
    }
    return owned;
}

// Store unique, non-null targets supplied by the caller.
// Kept for compatibility; automatic toAttack() does not use this list.
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
vector<Territory*> Player::toAttack() const {
    vector<Territory*> targets;
    for (Territory* territory : toDefend()) {
        for (Territory* neighbour : territory->getNeighbours()) {
            if (neighbour && neighbour->getOwner() != this
                    && find(targets.begin(), targets.end(), neighbour)
                           == targets.end()) {
                targets.push_back(neighbour);
            }
        }
    }
    return targets;
}

// Attach/read the player's borrowed card wallet.
void Player::setHand(Hand* playerHand) { hand = playerHand; }
Hand* Player::getHand() const { return hand; }

// Attach/read the player's borrowed order notebook.
void Player::setOrdersList(OrdersList* playerOrders) { orders = playerOrders; }
OrdersList* Player::getOrdersList() const { return orders; }

// Report actual ownership rather than counting shared references as owned.
ostream& operator<<(ostream& out, const Player& player) {
    out << "Player: " << *player.name
        << " | owned territories: " << player.toDefend().size()
        << " | attack targets: " << player.toAttack().size()
        << " | hand: " << (player.hand ? "attached" : "not attached")
        << " | orders: " << (player.orders ? "attached" : "not attached");
    return out;
}
