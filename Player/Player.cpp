#include "Player.h"
#include "../Map/Map.h"
#include "../Orders/Orders.h"
#include "../Cards/Cards.h"

#include <algorithm>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <limits>

// Use standard library names directly, such as string, vector, and cout.
using namespace std;

// A default player starts with an empty command folder.
Player::Player() : Player("Unnamed player") {}

// Temporary smart pointers clean up automatically if an allocation fails.
// release() transfers responsibility for cleanup to Player's destructor.
Player::Player(const string& playerName)
    : name(nullptr), territories(nullptr), attackTargets(nullptr),
      hand(nullptr), orders(nullptr), reservedArmies(nullptr) {
    auto newName = make_unique<string>(playerName);
    auto newTerritories = make_unique<vector<Territory*>>();
    auto newTargets = make_unique<vector<Territory*>>();
    auto newReservations = make_unique<map<Territory*, int>>();
    name = newName.release();
    territories = newTerritories.release();
    attackTargets = newTargets.release();
    reservedArmies = newReservations.release();
}

// Copy the folder's lists, not the map's Territory objects.
// All borrowed pointers remain shared; copying never changes territory owners.
Player::Player(const Player& other)
    : name(nullptr), territories(nullptr), attackTargets(nullptr),
      hand(nullptr), orders(nullptr), reservedArmies(nullptr) {
    // Keep ownership local until every allocation and copy succeeds.
    // If any copy throws, these smart pointers release all partial state.
    auto newName = make_unique<string>(*other.name);
    auto newTerritories = make_unique<vector<Territory*>>(*other.territories);
    auto newTargets = make_unique<vector<Territory*>>(*other.attackTargets);
    auto newReservations = make_unique<map<Territory*, int>>(*other.reservedArmies);
    name = newName.release();
    territories = newTerritories.release();
    attackTargets = newTargets.release();
    reservedArmies = newReservations.release();
    hand = other.hand;
    orders = other.orders;
    reinforcementPool = other.reinforcementPool;
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
        swap(reinforcementPool, copy.reinforcementPool);
        swap(reservedArmies, copy.reservedArmies);
    }
    return *this;
}

// Delete only what Player allocated. Borrowed objects belong elsewhere.
Player::~Player() {
    delete name;
    delete territories;
    delete attackTargets;
    delete reservedArmies;
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

// The list owns the new order; Player continues to borrow the list itself.
void Player::issueOrder(const string& orderType) {
    if (!orders) {
        throw logic_error("Attach an OrdersList before issuing an order.");
    }
    if (orders->getOrdersListSize() == 0) reservedArmies->clear();
    Territory* movementSource = nullptr;
    unique_ptr<Orders> order;
    if (orderType == "deploy") {
        const auto defend = toDefend();
        if (defend.empty()) throw logic_error("Deploy needs an owned territory.");
        order = make_unique<Deploy>(this, defend.front(), getReinforcementPool());
    } else if (orderType == "advance") {
        for (Territory* source : toDefend()) {
            if (availableArmies(source) < 1) continue;
            for (Territory* target : source->getNeighbours()) {
                if (source == target) continue;
                movementSource = source;
                order = make_unique<Advance>(this, source, target, 1);
                break;
            }
            if (order) break;
        }
        if (!order) throw logic_error("Advance needs an owned army and an adjacent target.");
    } else if (orderType == "bomb") {
        for (Territory* target : toAttack()) {
            if (!target->getOwner()) continue;
            order = make_unique<Bomb>(this, target);
            break;
        }
        if (!order) throw logic_error("Bomb needs an adjacent enemy territory.");
    } else if (orderType == "blockade") {
        const auto defend = toDefend();
        if (defend.empty()) throw logic_error("Blockade needs an owned territory.");
        order = make_unique<Blockade>(this, defend.front());
    } else if (orderType == "airlift") {
        const auto defend = toDefend();
        for (Territory* source : defend) {
            if (availableArmies(source) < 1) continue;
            for (Territory* target : defend) {
                if (source == target) continue;
                movementSource = source;
                order = make_unique<Airlift>(this, source, target, 1);
                break;
            }
            if (order) break;
        }
        if (!order) throw logic_error("Airlift needs two owned territories and an army to move.");
    } else if (orderType == "negotiate") {
        // No global roster is available here; choose an enemy known through
        // adjacent territories rather than inventing a target player.
        for (Territory* target : toAttack()) {
            if (!target->getOwner()) continue;
            order = make_unique<Negotiate>(this, target->getOwner());
            break;
        }
        if (!order) throw logic_error("Negotiate needs a known enemy player.");
    } else {
        throw invalid_argument("Unknown order type: " + orderType);
    }
    // If insertion throws, the temporary pointer cleans up automatically.
    if (movementSource) reservedArmies->try_emplace(movementSource, 0);
    orders->add(order.get());
    order.release();
    if (movementSource) ++reservedArmies->at(movementSource);
}

// Report actual ownership rather than counting shared references as owned.
ostream& operator<<(ostream& out, const Player& player) {
    out << "Player: " << *player.name
        << " | owned territories: " << player.toDefend().size()
        << " | attack targets: " << player.toAttack().size()
        << " | hand: " << (player.hand ? "attached" : "not attached")
        << " | orders: " << (player.orders ? "attached" : "not attached");
    return out;
}

int Player::getReinforcementPool() const { return reinforcementPool; }

void Player::addReinforcements(int armies) {
    if (armies < 0 || armies > numeric_limits<int>::max() - reinforcementPool)
        throw invalid_argument("Invalid reinforcement addition.");
    reinforcementPool += armies;
}

// Part 4's valid Deploy execution must spend the real pool exactly once.
void Player::spendReinforcements(int armies) {
    if (armies < 0 || armies > reinforcementPool)
        throw invalid_argument("Not enough reinforcement armies.");
    reinforcementPool -= armies;
}

int Player::availableArmies(Territory* territory) const {
    const auto reserved = reservedArmies->find(territory);
    return territory->getArmies() - (reserved == reservedArmies->end() ? 0 : reserved->second);
}

bool Player::issueOrder(Deck* deck, int& armiesToDeploy, int& nextOrder) {
    if (!orders) throw logic_error("Attach an OrdersList before issuing an order.");
    if (orders->getOrdersListSize() == 0) reservedArmies->clear();
    const auto defend = toDefend();
    if (defend.empty()) return false;
    if (armiesToDeploy > 0) {
        auto order = make_unique<Deploy>(this, defend.front(), armiesToDeploy);
        orders->add(order.get());
        order.release(); // OrdersList owns it; no pool deduction during issuing.
        // Reserve for this phase; only execution may spend the real pool.
        cout << getName() << " queues Deploy " << armiesToDeploy << " -> "
             << defend.front()->getName() << '\n';
        armiesToDeploy = 0;
        return true;
    }
    // A simple finite policy: one friendly Advance, one enemy Advance, cards.
    while (nextOrder < 2) {
        const bool attack = nextOrder == 1;
        const auto targets = attack ? toAttack() : defend;
        for (Territory* source : defend) {
            if (availableArmies(source) < 1) continue;
            for (Territory* target : targets) {
                const auto& neighbours = source->getNeighbours();
                if (source == target || find(neighbours.begin(), neighbours.end(), target) == neighbours.end()) continue;
                const int amount = attack ? availableArmies(source) : 1;
                auto order = make_unique<Advance>(this, source, target, amount);
                reservedArmies->try_emplace(source, 0);
                orders->add(order.get());
                order.release();
                reservedArmies->at(source) += amount;
                ++nextOrder;
                cout << getName() << " queues " << (attack ? "attack" : "friendly")
                     << " Advance " << amount << ": " << source->getName() << " -> " << target->getName()
                     << '\n';
                return true;
            }
        }
        ++nextOrder;
    }
    if (hand && hand->getSize() > 0) {
        if (!deck || !orders) throw logic_error("Card issuance needs a Deck and OrdersList.");
        for (Card* card : *hand->getCards()) {
            // Successful play removes this card; return before advancing the
            // invalidated iterator. Unusable cards stay for another turn.
            if (card->play(*hand, *deck, *orders, *this)) return true;
        }
    }
    return false;
}
