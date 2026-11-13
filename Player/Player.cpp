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

//player constrcutor
// Temporary smart pointers clean up automatically if an allocation fails.
// release() transfers responsibility for cleanup to Player's destructor.
Player::Player(const string& playerName)
    : name(nullptr), territories(nullptr), attackTargets(nullptr),
      hand(nullptr), orders(nullptr), orderFactory(nullptr), deck(nullptr),
      reinforcementPool(0), committedArmies(0), decisionStage(0),
      cardsRemaining(0), issuingStarted(false), friendlySource(nullptr) {
    auto newName = make_unique<string>(playerName);
    auto newTerritories = make_unique<vector<Territory*>>();
    auto newTargets = make_unique<vector<Territory*>>();
    name = newName.release();
    territories = newTerritories.release();
    attackTargets = newTargets.release();
}

//copy construcotr
// Copy the folder's lists, not the map's Territory objects.
// All borrowed pointers remain shared; copying never changes territory owners.
Player::Player(const Player& other)
    : name(nullptr), territories(nullptr), attackTargets(nullptr),
      hand(nullptr), orders(nullptr), orderFactory(other.orderFactory), deck(other.deck),
      reinforcementPool(other.reinforcementPool), committedArmies(other.committedArmies),
      decisionStage(other.decisionStage), cardsRemaining(other.cardsRemaining),
      issuingStarted(other.issuingStarted), friendlySource(other.friendlySource) {
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

//assignement operator
// Copy first, then swap: if copying fails, the original player stays intact.
Player& Player::operator=(const Player& other) {
    //prevent unecessary self assignement
    if (this != &other) {
        Player copy(other);
        swap(name, copy.name);
        swap(territories, copy.territories);
        swap(attackTargets, copy.attackTargets);
        swap(hand, copy.hand);
        swap(orders, copy.orders);
        swap(orderFactory, copy.orderFactory);
        swap(deck, copy.deck);
        swap(reinforcementPool, copy.reinforcementPool);
        swap(committedArmies, copy.committedArmies);
        swap(decisionStage, copy.decisionStage);
        swap(cardsRemaining, copy.cardsRemaining);
        swap(issuingStarted, copy.issuingStarted);
        swap(friendlySource, copy.friendlySource);
    }
    return *this;
}

//deconstrcutor
// Delete only what Player allocated. Borrowed objects belong elsewhere.
Player::~Player() {
    delete name;
    delete territories;
    delete attackTargets;
}

// A const reference lets the caller read the name without copying/changing it.
const string& Player::getName() const { return *name; }

// Register a territory once, then transfer it from its previous owner.
//add territory to a player
//if beloungs to another player it deletes it from the old player
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

// "The toAttack() method examines the neighbours of the territories owned by the player. If a neighbouring territory
//is not owned by the player, it's considered a possible attack target. I also avoid adding duplicates."
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
    unique_ptr<Orders> order;
    if (orderType == "deploy") {
        order = make_unique<Deploy>();
    } else if (orderType == "advance") {
        order = make_unique<Advance>();
    } else if (orderType == "bomb") {
        order = make_unique<Bomb>();
    } else if (orderType == "blockade") {
        order = make_unique<Blockade>();
    } else if (orderType == "airlift") {
        order = make_unique<Airlift>();
    } else if (orderType == "negotiate") {
        order = make_unique<Negotiate>();
    } else {
        throw invalid_argument("Unknown order type: " + orderType);
    }
    // If insertion throws, the temporary pointer cleans up automatically.
    orders->add(order.get());
    order.release();
}

// Report actual ownership rather than counting shared references as owned.
ostream& operator<<(ostream& out, const Player& player) {
    out << "Player: " << *player.name
        << " | owned territories: " << player.toDefend().size()
        << " | attack targets: " << player.toAttack().size()
        << " | hand: " << (player.hand ? "attached" : "not attached")
        << " | orders: " << (player.orders ? "attached" : "not attached")
        << " | reinforcement pool: " << player.reinforcementPool;
    return out;
}

// Pool changes never deploy armies themselves; Part 4 enacts Deploy effects.
int Player::getReinforcementPool() const { return reinforcementPool; }

int Player::getUncommittedArmies() const {
    return max(0, reinforcementPool - committedArmies);
}

void Player::addReinforcements(int armies) {
    if (armies < 0 || armies > numeric_limits<int>::max() - reinforcementPool) {
        throw invalid_argument("Invalid reinforcement addition.");
    }
    reinforcementPool += armies;
}

void Player::spendReinforcements(int armies) {
    if (armies < 0 || armies > reinforcementPool) {
        throw invalid_argument("Cannot spend more armies than the reinforcement pool.");
    }
    reinforcementPool -= armies;
    committedArmies = max(0, committedArmies - armies);
}

void Player::setOrderFactory(OrderFactory factory) { orderFactory = factory; }
bool Player::hasOrderFactory() const { return orderFactory != nullptr; }

void Player::beginOrderIssuing(Deck* sharedDeck) {
    if (!orders) {
        throw logic_error("Attach an OrdersList before issuing orders.");
    }
    if (orders->getOrdersListSize() != 0) {
        throw logic_error("Finish pending orders before starting a new issuing phase.");
    }
    deck = sharedDeck;
    committedArmies = 0;
    decisionStage = 0;
    cardsRemaining = hand ? hand->getSize() : 0;
    friendlySource = nullptr;
    issuingStarted = true;
}

// Invalid Deploy orders leave their armies in the real pool for the next turn.
void Player::endOrderIssuing() {
    committedArmies = 0;
    issuingStarted = false;
    friendlySource = nullptr;
}

void Player::queueOrder(const string& type, Territory* source, Territory* target,
                        Player* targetPlayer, int armies) {
    if (!orderFactory) {
        throw logic_error("Part 4 integration required: install Player::setOrderFactory().");
    }
    unique_ptr<Orders> order(orderFactory(type, *this, source, target, targetPlayer, armies));
    if (!order) {
        throw logic_error("The order factory returned null for " + type + ".");
    }
    orders->add(order.get());
    order.release();
}

bool Player::issueOrder() {
    if (!issuingStarted) {
        beginOrderIssuing(deck);
    }
    const vector<Territory*> defend = toDefend();
    if (defend.empty()) {
        decisionStage = 3;
        cardsRemaining = 0;
        return false;
    }
    // Commit only after successful insertion. The actual pool remains intact
    // until the order's execute() spends it; no double deduction occurs.
    const int available = getUncommittedArmies();
    if (available > 0) {
        queueOrder("deploy", nullptr, defend.front(), nullptr, available);
        committedArmies += available;
        return true;
    }
    if (decisionStage == 0) {
        for (Territory* source : defend) {
            if (source->getArmies() <= 0) continue;
            for (Territory* target : defend) {
                if (source != target && find(source->getNeighbours().begin(),
                        source->getNeighbours().end(), target) != source->getNeighbours().end()) {
                    queueOrder("advance", source, target, nullptr, 1);
                    friendlySource = source;
                    decisionStage = 1;
                    return true;
                }
            }
        }
        decisionStage = 1;
    }
    if (decisionStage == 1) {
        for (Territory* target : toAttack()) {
            // Unowned territories are not enemy-player attack targets.
            if (!target->getOwner()) continue;
            for (Territory* source : defend) {
                const int reserved = source == friendlySource ? 1 : 0;
                if (source->getArmies() > reserved && find(source->getNeighbours().begin(),
                        source->getNeighbours().end(), target) != source->getNeighbours().end()) {
                    queueOrder("advance", source, target, nullptr, 1);
                    decisionStage = 2;
                    return true;
                }
            }
        }
        decisionStage = 2;
    }
    if (decisionStage == 2 && hand && cardsRemaining > 0) {
        if (!deck) {
            throw logic_error("Attach the shared Deck to issue card orders.");
        }
        // Snapshot budget prevents an adapter or new card reward extending this
        // phase indefinitely. Unusable cards stay in the hand for later turns.
        const size_t limit = min(static_cast<size_t>(cardsRemaining), hand->getCards()->size());
        for (size_t i = 0; i < limit; ++i) {
            Card* card = (*hand->getCards())[i];
            if (!card) continue;
            string type;
            Territory* source = nullptr;
            Territory* target = defend.front();
            Player* targetPlayer = nullptr;
            int armies = 0;
            switch (card->getType()) {
                case CardType::Bomb:
                case CardType::Diplomacy: {
                    target = nullptr;
                    for (Territory* enemy : toAttack()) {
                        if (enemy->getOwner()) { target = enemy; break; }
                    }
                    if (!target) continue;
                    if (card->getType() == CardType::Bomb) type = "bomb";
                    else { type = "negotiate"; targetPlayer = target->getOwner(); target = nullptr; }
                    break;
                }
                case CardType::Blockade: type = "blockade"; break;
                case CardType::Airlift:
                    if (defend.size() < 2) continue;
                    for (Territory* owned : defend) {
                        if (owned->getArmies() > 0) { source = owned; break; }
                    }
                    if (!source) continue;
                    target = defend.front() == source ? defend[1] : defend.front();
                    armies = 1;
                    type = "airlift";
                    break;
                case CardType::Reinforcement:
                    // The handout does not specify this card's bonus. The
                    // adapter supplies its order and effect, never a guessed bonus.
                    type = "reinforcement";
                    break;
            }
            queueOrder(type, source, target, targetPlayer, armies);
            try {
                // Card::play() cannot accept a contextual order. Transfer the
                // actual card using existing APIs instead of its placeholder
                // factory. Deck owns it after removal from Hand; no delete this.
                deck->addCard(card);
            } catch (...) {
                orders->remove(static_cast<int>(orders->getOrdersListSize() - 1));
                throw;
            }
            hand->removeCard(card);
            --cardsRemaining;
            return true;
        }
    }
    decisionStage = 3;
    cardsRemaining = 0;
    return false;
}
