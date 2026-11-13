#include "GameEngine.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include "../Map/Map.h"
#include "../Player/Player.h"
#include "../Orders/Orders.h"
#include "../Cards/Cards.h"

/**
 * Console driver for the GameEngine. The user types commands that trigger
 * state transitions as depicted in the assignment's state diagram.
 * Invalid commands are rejected with an error message and do not change the state.
 * The program ends when the "end" command is entered in the "win" state.
 */
int runPart3Tests();

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--part3") return runPart3Tests();
    if (argc != 1) {
        std::cerr << "Usage: " << argv[0] << " [--part3]\n";
        return 1;
    }
    GameEngine engine;

    std::cout << "=== Warzone Game Engine ===\n"
              << "Type a command to move through the game states.\n\n";

    std::string input;
    while (!engine.isFinished()) {
        std::cout << "Current state: " << engine.getCurrentStateName() << "\n";
        std::cout << "Valid commands:";
        for (const std::string& command : engine.getValidCommands()) {
            std::cout << " " << command;
        }
        std::cout << "\n> ";

        if (!std::getline(std::cin, input)) {
            std::cout << "\nInput closed. Exiting." << std::endl;
            return 0;
        }
        engine.processCommand(input);
        std::cout << std::endl;
    }

    std::cout << "Game over. Thanks for playing!" << std::endl;
    return 0;
}

namespace {

// These doubles are deliberately confined to the driver. They do NOT implement
// Part 4 combat rules. The final loop demonstration uses a scripted capture.
std::vector<std::string> issuedTrace;
std::vector<std::string> executionTrace;
bool simulateEffects = false;
int deployExecutions = 0;
int captureAfterDeployments = 0;
int validationCalls = 0;
bool throwOnExecution = false;

/** Throw on a failed expectation even when compiled with NDEBUG. */
void expect(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error("FAIL: " + message);
}

/** Reset isolated test instrumentation, without changing production classes. */
void resetTrace() {
    issuedTrace.clear();
    executionTrace.clear();
    simulateEffects = false;
    deployExecutions = 0;
    captureAfterDeployments = 0;
    validationCalls = 0;
    throwOnExecution = false;
}

/** Print the exact order trace so the marker can inspect scheduling directly. */
void showTrace(const std::string& label, const std::vector<std::string>& trace) {
    std::cout << label;
    for (const std::string& entry : trace) std::cout << " " << entry;
    std::cout << '\n';
}

/** Recording order with optional scripted army movement/capture for fixtures. */
class DemonstrationOrder : public Orders {
public:
    DemonstrationOrder(const std::string& type, const std::string& label,
                       Player* issuer, Territory* source, Territory* target, int armies);
    DemonstrationOrder(const DemonstrationOrder& other);
    DemonstrationOrder& operator=(const DemonstrationOrder& other);
    ~DemonstrationOrder() override;
    std::string getName() const override;
    bool validate() const override;
    void execute() override;
    Orders* clone() const override;
    Territory* getSource() const;
    Territory* getTarget() const;
    int getArmies() const;
private:
    std::string type;
    std::string label;
    Player* issuer;
    Territory* source;
    Territory* target;
    int armies;
    std::ostream& print(std::ostream& out) const override;
};

DemonstrationOrder::DemonstrationOrder(const std::string& type, const std::string& label,
        Player* issuer, Territory* source, Territory* target, int armies)
    : type(type), label(label), issuer(issuer), source(source), target(target), armies(armies) {}
DemonstrationOrder::DemonstrationOrder(const DemonstrationOrder& other) = default;
DemonstrationOrder& DemonstrationOrder::operator=(const DemonstrationOrder& other) = default;
DemonstrationOrder::~DemonstrationOrder() = default;
std::string DemonstrationOrder::getName() const { return type; }
Territory* DemonstrationOrder::getSource() const { return source; }
Territory* DemonstrationOrder::getTarget() const { return target; }
int DemonstrationOrder::getArmies() const { return armies; }
Orders* DemonstrationOrder::clone() const { return new DemonstrationOrder(*this); }
std::ostream& DemonstrationOrder::print(std::ostream& out) const { return out << label; }

bool DemonstrationOrder::validate() const {
    ++validationCalls;
    if (!simulateEffects) return true;
    if (type == "Deploy") {
        return issuer && target && target->getOwner() == issuer
            && armies > 0 && issuer->getReinforcementPool() >= armies;
    }
    return true;
}

void DemonstrationOrder::execute() {
    if (throwOnExecution) throw std::runtime_error("Scripted execution failure");
    executionTrace.push_back(label);
    if (!validate() || !simulateEffects) return;
    if (type == "Deploy") {
        issuer->spendReinforcements(armies);
        target->setArmies(target->getArmies() + armies);
        ++deployExecutions;
    } else if (type == "Advance" && source && target && source->getOwner() == issuer
               && source->getArmies() >= armies) {
        if (target->getOwner() == issuer) {
            source->setArmies(source->getArmies() - armies);
            target->setArmies(target->getArmies() + armies);
        } else if (deployExecutions >= captureAfterDeployments) {
            // SCRIPTED TEST EFFECT: deliberately no battle probabilities.
            source->setArmies(source->getArmies() - armies);
            issuer->addTerritory(target);
            target->setArmies(armies);
        }
    }
}

/** Part 4 factory contract exercised without pretending its effects exist. */
Orders* recordingFactory(const std::string& kind, Player& player, Territory* source,
                        Territory* target, Player* targetPlayer, int armies) {
    std::string type = kind;
    if (kind == "reinforcement") type = "deploy";
    type[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(type[0])));
    if (kind == "negotiate") expect(targetPlayer && targetPlayer != &player, "diplomacy target");
    const std::string label = player.getName() + ":" + kind;
    issuedTrace.push_back(label);
    return new DemonstrationOrder(type, label, &player, source, target, armies);
}

/** An adapter failure must leave both pool commitments and list unchanged. */
Orders* nullFactory(const std::string&, Player&, Territory*, Territory*, Player*, int) {
    return nullptr;
}

/** Build a connected chain; Map owns territories and its single continent. */
std::unique_ptr<Map> makeMap(int count, int bonus = 0) {
    auto map = std::make_unique<Map>();
    auto continent = std::make_unique<Continent>("Test continent", bonus, "blue");
    Continent* group = continent.get();
    map->addContinent(group);
    continent.release();
    for (int i = 0; i < count; ++i) {
        auto territory = std::make_unique<Territory>(i + 1, "T" + std::to_string(i + 1), group);
        map->addCountry(territory.get());
        territory.release();
        Territory* current = map->getCountries().back();
        group->addTerritory(current);
        if (i > 0) {
            Territory* previous = map->getCountries()[i - 1];
            previous->addNeighbour(current);
            current->addNeighbour(previous);
        }
    }
    return map;
}

/** Demonstration 1: floor/minimum, accumulation and complete-continent bonuses. */
void testReinforcement() {
    std::cout << "\n1. Reinforcement calculations (real Part 3 calculation)\n";
    for (int count : {1, 8, 9, 12}) {
        auto map = makeMap(count + 1, 6);
        Player a("A"), b("B");
        for (int i = 0; i < count; ++i) a.addTerritory(map->getCountries()[i]);
        b.addTerritory(map->getCountries().back());
        a.addReinforcements(50);
        GameEngine engine;
        engine.setGameContext(map.get(), {&a, &b});
        engine.reinforcementPhase();
        expect(a.getReinforcementPool() == 50 + std::max(3, count / 3), "floor/minimum and accumulation");
        expect(map->getCountries()[0]->getArmies() == 0, "reinforcement does not deploy");
    }
    auto map = makeMap(2, 5);
    Player a("A"), b("B"), eliminated("Eliminated");
    a.addTerritory(map->getCountries()[0]);
    a.addTerritory(map->getCountries()[1]);
    auto extra = std::make_unique<Continent>("Second", 2, "red");
    Continent* group = extra.get();
    map->addContinent(group);
    extra.release();
    auto territory = std::make_unique<Territory>(3, "T3", group);
    Territory* t3 = territory.get();
    map->addCountry(t3);
    territory.release();
    group->addTerritory(t3);
    a.addTerritory(t3);
    GameEngine engine;
    engine.setGameContext(map.get(), {&a, &b, &eliminated});
    engine.reinforcementPhase();
    expect(a.getReinforcementPool() == 10, "minimum plus two continent bonuses");
    expect(eliminated.getReinforcementPool() == 0, "zero-territory player gets nothing");
    b.addTerritory(t3);
    engine.reinforcementPhase();
    expect(a.getReinforcementPool() == 18, "loss of continent and repeated awards");
    Player copy(a);
    Player assigned;
    assigned = a;
    copy.addReinforcements(1);
    expect(a.getReinforcementPool() == 18 && copy.getReinforcementPool() == 19
           && assigned.getReinforcementPool() == 18, "independent copied pool");
    bool rejected = false;
    try { a.spendReinforcements(19); } catch (const std::invalid_argument&) { rejected = true; }
    expect(rejected && a.getReinforcementPool() == 18, "pool cannot overspend");
}

/** Demonstrations 2-4: real decisions, contextual test orders, real card transfer. */
void testPlayerDecisions() {
    std::cout << "\n2-4. Deploy-only, friendly/enemy Advance and cards (recording orders)\n";
    resetTrace();
    auto map = makeMap(3);
    Deck deck;
    Hand hand;
    OrdersList list;
    Player a("A"), b("B");
    a.addTerritory(map->getCountries()[0]);
    a.addTerritory(map->getCountries()[1]);
    b.addTerritory(map->getCountries()[2]);
    map->getCountries()[0]->setArmies(2);
    map->getCountries()[1]->setArmies(2);
    a.setOrdersList(&list);
    a.setHand(&hand);
    a.setOrderFactory(recordingFactory);
    a.addReinforcements(5);
    const CardType types[] = {CardType::Bomb, CardType::Blockade, CardType::Airlift,
                             CardType::Diplomacy, CardType::Reinforcement};
    for (CardType type : types) hand.addCard(new Card(type));
    a.beginOrderIssuing(&deck);
    expect(a.issueOrder(), "first decision deploys");
    auto deploy = dynamic_cast<DemonstrationOrder*>(list.getOrder(0));
    expect(deploy && deploy->getName() == "Deploy" && deploy->getArmies() == 5
           && deploy->getTarget() == map->getCountries()[0], "all pool armies committed to defended territory");
    expect(a.getReinforcementPool() == 5 && a.getUncommittedArmies() == 0, "issuing reserves without spending");
    expect(hand.getSize() == 5 && validationCalls == 0, "no cards or validation before Deploy commitment");
    std::cout << "Deploy: target " << deploy->getTarget()->getName()
              << ", armies " << deploy->getArmies() << "; actual pool "
              << a.getReinforcementPool() << ", uncommitted " << a.getUncommittedArmies() << '\n';
    expect(a.issueOrder(), "friendly advance issued");
    auto friendly = dynamic_cast<DemonstrationOrder*>(list.getOrder(1));
    expect(friendly && friendly->getSource() == map->getCountries()[0]
           && friendly->getTarget() == map->getCountries()[1], "defend list drives friendly move");
    expect(a.issueOrder(), "enemy advance issued");
    auto attack = dynamic_cast<DemonstrationOrder*>(list.getOrder(2));
    expect(attack && attack->getSource() == map->getCountries()[1]
           && attack->getTarget() == map->getCountries()[2], "attack list drives enemy move");
    std::cout << "Friendly Advance: " << friendly->getSource()->getName() << " -> "
              << friendly->getTarget()->getName() << "; attack Advance: "
              << attack->getSource()->getName() << " -> " << attack->getTarget()->getName() << '\n';
    for (int i = 0; i < 5; ++i) {
        const size_t before = list.getOrdersListSize();
        expect(a.issueOrder() && list.getOrdersListSize() == before + 1, "one card order per call");
    }
    expect(!a.issueOrder() && !a.issueOrder(), "issuance terminates reliably");
    expect(hand.getSize() == 0 && deck.getSize() == 5, "played cards transfer to shared deck");
    expect(validationCalls == 0 && executionTrace.empty(), "issuance never validates or executes");
    expect(issuedTrace == std::vector<std::string>({"A:deploy", "A:advance", "A:advance",
        "A:bomb", "A:blockade", "A:airlift", "A:negotiate", "A:reinforcement"}), "all card decisions");
    showTrace("Issued:", issuedTrace);
    std::cout << "Cards: hand " << hand.getSize() << ", deck " << deck.getSize()
              << "; validation/execution calls during issuance: " << validationCalls << '\n';

    // Adapters are required rather than silently accepting empty placeholders.
    OrdersList missingList;
    Player missing("Missing");
    missing.setOrdersList(&missingList);
    missing.addTerritory(map->getCountries()[0]);
    missing.addReinforcements(3);
    bool rejected = false;
    try { missing.issueOrder(); } catch (const std::logic_error&) { rejected = true; }
    expect(rejected && missingList.getOrdersListSize() == 0, "missing factory fails explicitly");
    missing.setOrderFactory(nullFactory);
    rejected = false;
    try { missing.issueOrder(); } catch (const std::logic_error&) { rejected = true; }
    expect(rejected && missing.getUncommittedArmies() == 3, "failed factory does not commit armies");
}

/** Test unequal issuance lengths in the startup order B,A,C. */
void testRoundRobinIssuing() {
    std::cout << "\nRound-robin issuing (recording factory)\n";
    resetTrace();
    auto map = makeMap(3);
    Deck deck;
    Hand hand;
    OrdersList la, lb, lc;
    Player a("A"), b("B"), c("C");
    a.addTerritory(map->getCountries()[0]);
    b.addTerritory(map->getCountries()[1]);
    c.addTerritory(map->getCountries()[2]);
    a.setOrdersList(&la); b.setOrdersList(&lb); c.setOrdersList(&lc);
    for (Player* p : {&a, &b, &c}) p->setOrderFactory(recordingFactory);
    a.addReinforcements(3); b.addReinforcements(3);
    a.setHand(&hand);
    hand.addCard(new Card(CardType::Bomb));
    GameEngine engine;
    engine.setGameContext(map.get(), {&b, &a, &c}, &deck);
    engine.issueOrdersPhase();
    expect(issuedTrace == std::vector<std::string>({"B:deploy", "A:deploy", "A:bomb"}),
           "round-robin respects startup order and skips finished players");
    showTrace("Issued in startup order B,A,C:", issuedTrace);
    expect(engine.getActivePlayers() == std::vector<Player*>({&b, &a, &c}), "roster remains ordered");
    engine.executeOrdersPhase();
    expect(la.getOrdersListSize() == 0 && lb.getOrdersListSize() == 0
           && lc.getOrdersListSize() == 0, "all issued orders consumed");
}

/** Exercise Deploy discovery behind other orders and empty/unequal queues. */
void testExecutionScheduling() {
    std::cout << "\nDeploy-first and round-robin execution (recording orders)\n";
    resetTrace();
    auto map = makeMap(3);
    OrdersList la, lb, lc;
    Player a("A"), b("B"), c("C");
    a.addTerritory(map->getCountries()[0]); b.addTerritory(map->getCountries()[1]);
    c.addTerritory(map->getCountries()[2]);
    a.setOrdersList(&la); b.setOrdersList(&lb); c.setOrdersList(&lc);
    la.add(new DemonstrationOrder("Advance", "A1", &a, nullptr, nullptr, 0));
    la.add(new DemonstrationOrder("Deploy", "A2", &a, nullptr, nullptr, 0));
    la.add(new DemonstrationOrder("Bomb", "A3", &a, nullptr, nullptr, 0));
    lb.add(new DemonstrationOrder("Deploy", "B1", &b, nullptr, nullptr, 0));
    lb.add(new DemonstrationOrder("Advance", "B2", &b, nullptr, nullptr, 0));
    lb.add(new DemonstrationOrder("Deploy", "B3", &b, nullptr, nullptr, 0));
    GameEngine engine;
    engine.setGameContext(map.get(), {&a, &b, &c});
    engine.executeOrdersPhase();
    expect(executionTrace == std::vector<std::string>({"A2", "B1", "B3", "A1", "B2", "A3"}),
           "Deploy-first round-robin trace");
    showTrace("Executed (A2,B1,B3 are Deploy):", executionTrace);
    expect(validationCalls == 6, "execution delegated exactly once per order");
    engine.executeOrdersPhase();
    expect(executionTrace.size() == 6, "empty queues handled normally");
    expect(engine.getCurrentStateName() == "assign reinforcement", "returns to reinforcement");

    // A failed real Deploy is removed, while the unspent pool is reusable.
    simulateEffects = true;
    a.addReinforcements(3);
    la.add(new DemonstrationOrder("Deploy", "invalid", &a, nullptr, map->getCountries()[1], 3));
    engine.executeOrdersPhase();
    expect(la.getOrdersListSize() == 0 && a.getReinforcementPool() == 3, "invalid order consumed without spending");

    la.add(new DemonstrationOrder("Deploy", "throwing", &a, nullptr, map->getCountries()[0], 3));
    Orders* pending = la.getOrder(0);
    throwOnExecution = true;
    bool propagated = false;
    try { engine.executeOrdersPhase(); } catch (const std::runtime_error&) { propagated = true; }
    expect(propagated && la.getOrdersListSize() == 1 && la.getOrder(0) == pending,
           "execution exception leaves the order owned by its list");
    throwOnExecution = false;
    engine.executeOrdersPhase();
    expect(la.getOrdersListSize() == 0 && a.getReinforcementPool() == 0, "order can be retried explicitly");
}

/** Demonstrations 5-6: authoritative ownership, stable elimination, immediate win. */
void testEliminationAndVictory() {
    std::cout << "\n5-6. Elimination and victory (controlled ownership fixture)\n";
    resetTrace();
    auto map = makeMap(3);
    OrdersList la, lb, lc;
    Player a("A"), b("B"), c("C");
    a.setOrdersList(&la); b.setOrdersList(&lb); c.setOrdersList(&lc);
    a.addTerritory(map->getCountries()[0]); b.addTerritory(map->getCountries()[1]);
    c.addTerritory(map->getCountries()[2]);
    GameEngine engine;
    engine.setGameContext(map.get(), {&b, &c, &a});
    expect(!engine.checkVictory(), "no premature victory");
    a.addTerritory(map->getCountries()[2]);
    engine.eliminatePlayers();
    expect(engine.getActivePlayers() == std::vector<Player*>({&b, &a}), "stable elimination");
    expect(c.toDefend().empty() && c.getName() == "C", "elimination does not delete Player");
    b.removeTerritory(map->getCountries()[1]);
    engine.eliminatePlayers();
    expect(!engine.checkVictory(), "one active player with unowned territory is not a winner");
    c.addTerritory(map->getCountries()[1]);
    expect(!engine.checkVictory(), "territory owned outside roster prevents victory");
    a.addTerritory(map->getCountries()[1]);
    expect(engine.checkVictory() == &a && engine.getCurrentStateName() == "win", "all map ownership wins");
    expect(!engine.isFinished(), "win returns control for replay/quit rather than exiting");

    Map empty;
    GameEngine emptyEngine;
    emptyEngine.setGameContext(&empty, {&a});
    expect(!emptyEngine.checkVictory(), "empty map is not victory");
}

/** Full phase loop with test-only effects: capture scripted on the second turn. */
void testRepeatedTurnsAndImmediateWin() {
    std::cout << "\nRepeated main loop and immediate victory (SCRIPTED capture, not Part 4 combat)\n";
    resetTrace();
    simulateEffects = true;
    captureAfterDeployments = 4;
    auto map = makeMap(2);
    OrdersList la, lb;
    Player a("A"), b("B");
    a.addTerritory(map->getCountries()[0]); b.addTerritory(map->getCountries()[1]);
    map->getCountries()[0]->setArmies(1);
    map->getCountries()[1]->setArmies(1);
    a.setOrdersList(&la); b.setOrdersList(&lb);
    a.setOrderFactory(recordingFactory); b.setOrderFactory(recordingFactory);
    GameEngine engine;
    engine.setGameContext(map.get(), {&a, &b});
    engine.mainGameLoop(3);
    expect(deployExecutions == 4, "two complete reinforcement/issuance cycles");
    expect(engine.getWinner() == &a && engine.getCurrentStateName() == "win", "loop stops at scripted capture");
    expect(executionTrace == std::vector<std::string>({"A:deploy", "B:deploy", "A:advance", "B:advance",
           "A:deploy", "B:deploy", "A:advance"}), "no order executes after winning capture");
    showTrace("Scripted two-turn execution:", executionTrace);
    expect(lb.getOrdersListSize() == 1 && engine.getActivePlayers() == std::vector<Player*>({&a}),
           "pending loser order remains safely owned; loser no longer participates");
    expect(a.getReinforcementPool() == 0 && b.getReinforcementPool() == 0, "Deploy spends once at execution");

    // A diagnostic limit never announces an invented winner.
    resetTrace();
    auto nonWinningMap = makeMap(2);
    OrdersList lx, ly;
    Player x("X"), y("Y");
    x.addTerritory(nonWinningMap->getCountries()[0]); y.addTerritory(nonWinningMap->getCountries()[1]);
    x.setOrdersList(&lx); y.setOrdersList(&ly);
    x.setOrderFactory(recordingFactory); y.setOrderFactory(recordingFactory);
    GameEngine limited;
    limited.setGameContext(nonWinningMap.get(), {&x, &y});
    bool stopped = false;
    try { limited.mainGameLoop(2); } catch (const std::runtime_error&) { stopped = true; }
    expect(stopped && !limited.getWinner() && x.getReinforcementPool() == 6,
           "bounded test runs two turns without claiming placeholder effects");
}

/** Regression: legacy commands/overload, graph copying and borrowed context. */
void testAssignmentOneAndIntegrationErrors() {
    std::cout << "\nAssignment 1 regressions and integration errors\n";
    GameEngine engine;
    expect(!engine.processCommand("win") && engine.getCurrentStateName() == "start", "invalid command rejected");
    for (const std::string& command : {" LOADMAP ", "validatemap", "addplayer", "assigncountries",
                                     "issueorder", "endissueorders", "win"}) {
        expect(engine.processCommand(command), "legacy graph command " + command);
    }
    GameEngine copy(engine), assigned;
    assigned = engine;
    expect(copy.getCurrentState() != engine.getCurrentState(), "engine graph deep copied");
    expect(copy.processCommand("play") && copy.getCurrentStateName() == "start", "copied transitions rewired");
    expect(assigned.processCommand("end") && assigned.isFinished(), "assigned graph exits");
    expect(engine.getCurrentStateName() == "win", "copies do not change original state");
    OrdersList legacy;
    Player player("Legacy");
    player.setOrdersList(&legacy);
    for (const std::string& kind : {"deploy", "advance", "bomb", "blockade", "airlift", "negotiate"}) {
        player.issueOrder(kind);
    }
    expect(legacy.getOrdersListSize() == 6, "legacy string-based overload preserved");
    OrdersList cloned(legacy), assignedList;
    assignedList = legacy;
    expect(cloned.getOrder(0) != legacy.getOrder(0) && assignedList.getOrdersListSize() == 6,
           "existing order-list cloning remains compatible");
    bool rejected = false;
    try { GameEngine missing; missing.mainGameLoop(); } catch (const std::logic_error&) { rejected = true; }
    expect(rejected, "missing startup context reported");
    auto map = makeMap(2);
    Player a("A"), b("B");
    a.addTerritory(map->getCountries()[0]); b.addTerritory(map->getCountries()[1]);
    engine.setGameContext(map.get(), {&a, &b});
    rejected = false;
    try { engine.mainGameLoop(); } catch (const std::logic_error&) { rejected = true; }
    expect(rejected && a.getReinforcementPool() == 0, "missing Part 4 fails before adding armies");
    rejected = false;
    try { engine.setGameContext(map.get(), {&a, &a}); } catch (const std::invalid_argument&) { rejected = true; }
    expect(rejected && engine.getActivePlayers().size() == 2, "duplicate roster rejected without replacing context");
    GameEngine contextCopy(engine);
    expect(contextCopy.getMap() == map.get() && contextCopy.getActivePlayers() == engine.getActivePlayers(),
           "game objects borrowed by engine copies");
    std::ostringstream output;
    output << player << engine;
    expect(!output.str().empty(), "stream operators remain usable");
}

} // namespace

/** Run explicit Part 3 demonstrations; failure yields a nonzero process status. */
int runPart3Tests() {
    std::cout << "=== Part 3 demonstrations ===\n"
              << "Order effects below are driver-only doubles, NOT completed Part 4 integration.\n";
    try {
        testReinforcement();
        testPlayerDecisions();
        testRoundRobinIssuing();
        testExecutionScheduling();
        testEliminationAndVictory();
        testRepeatedTurnsAndImmediateWin();
        testAssignmentOneAndIntegrationErrors();
        std::cout << "\nPASS: all Part 3 driver checks. Real combat/logging/startup remain external dependencies.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
