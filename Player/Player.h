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
class Orders;
class Deck;

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
    void issueOrder(const string& orderType);

    // Part 4 adapter: return a fresh owning order, without validation/execution.
    // Types are lowercase, including "reinforcement" for that card. Source,
    // territory target and player target are borrowed and may be null by type.
    // Deploy must spend the real pool during execute(), not during construction.
    using OrderFactory = Orders* (*)(const string&, Player&, Territory*, Territory*, Player*, int);
    void setOrderFactory(OrderFactory factory);
    bool hasOrderFactory() const;

    // A finite default policy: deploy the available pool, one friendly Advance,
    // one attack Advance, then eligible cards from the initial hand, then stop.
    // true means exactly one order was queued; false means finished this phase.
    // The no-argument Assignment 1 call remains valid, now using this policy.
    bool issueOrder();
    void beginOrderIssuing(Deck* deck = nullptr);
    void endOrderIssuing();
    int getReinforcementPool() const;
    int getUncommittedArmies() const;
    void addReinforcements(int armies);
    void spendReinforcements(int armies);

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
    // Gameplay connections are borrowed; counters are copied by value.
    OrderFactory orderFactory;
    Deck* deck;
    int reinforcementPool;
    int committedArmies;
    int decisionStage;
    int cardsRemaining;
    bool issuingStarted;
    Territory* friendlySource;

    // Builds through the teammate adapter and transfers ownership to the list.
    void queueOrder(const string& type, Territory* source, Territory* target,
                    Player* targetPlayer, int armies);
};

#endif
