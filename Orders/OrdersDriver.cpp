#include "Orders.h"
#include "../Player/Player.h"
#include "../Map/Map.h"
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace {
// Assertions stay enabled in release builds and return a failing exit status.
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void reject(Orders& order) {
    check(!order.validate(), "Invalid order was accepted");
    order.execute();
    check(order.stringToLog().find("Invalid") != std::string::npos, "Invalid effect was not recorded");
}
bool contains(const std::vector<Territory*>& list, Territory* territory) {
    return std::find(list.begin(), list.end(), territory) != list.end();
}
}

int main() {
    try {
        Orders::resetGameState();
        Player alice("Alice"), bob("Bob");
        Continent continent("Test", 5, "red");
        Territory home(1, "Home", &continent), friendly(2, "Friendly", &continent);
        Territory border(3, "Enemy border", &continent), remote(4, "Enemy remote", &continent);
        Territory airTarget(5, "Airlift target", &continent);
        Territory capture1(6, "Capture one", &continent), capture2(7, "Capture two", &continent);
        alice.addTerritory(&home); alice.addTerritory(&friendly); alice.addTerritory(&airTarget);
        bob.addTerritory(&border); bob.addTerritory(&remote);
        bob.addTerritory(&capture1); bob.addTerritory(&capture2);
        home.addNeighbour(&friendly); home.addNeighbour(&border);
        home.addNeighbour(&capture1); home.addNeighbour(&capture2);
        border.addNeighbour(&home);
        home.setArmies(10); friendly.setArmies(3); border.setArmies(8); airTarget.setArmies(2);
        alice.addReinforcements(6);

        std::cout << "1. Deploy: ownership, positive quantity, pool and single spending\n";
        Deploy deploy(&alice, &home, 6);
        check(deploy.validate(), "Valid Deploy rejected"); deploy.execute();
        check(home.getArmies() == 16 && alice.getReinforcementPool() == 0, "Deploy effect incorrect");
        Deploy copiedDeploy(deploy), assignedDeploy(&alice, &home, 1);
        assignedDeploy = copiedDeploy;
        reject(deploy); // Re-executing cannot spend the same pool twice.
        check(copiedDeploy.stringToLog().find("Deployed 6") != std::string::npos
              && assignedDeploy.stringToLog() == copiedDeploy.stringToLog(), "Order effects are not copied independently");
        Deploy wrongOwner(&alice, &border, 1), noPool(&alice, &home, 1);
        Deploy zeroDeploy(&alice, &home, 0), negativeDeploy(&alice, &home, -1);
        reject(wrongOwner); reject(noPool); reject(zeroDeploy); reject(negativeDeploy);
        check(home.getArmies() == 16 && border.getArmies() == 8, "Invalid Deploy changed armies");

        std::cout << "2. Advance: friendly movement and invalid ownership/adjacency/quantity\n";
        Advance move(&alice, &home, &friendly, 3);
        check(move.validate(), "Valid friendly Advance rejected"); move.execute();
        check(home.getArmies() == 13 && friendly.getArmies() == 6, "Friendly movement incorrect");
        Advance enemySource(&alice, &border, &home, 1), notAdjacent(&alice, &home, &remote, 1);
        Advance tooMany(&alice, &home, &friendly, 14), negativeMove(&alice, &home, &friendly, -1);
        Advance sameMove(&alice, &home, &home, 1), zeroMove(&alice, &home, &friendly, 0);
        reject(enemySource); reject(notAdjacent); reject(tooMany); reject(negativeMove); reject(sameMove); reject(zeroMove);
        check(home.getArmies() == 13 && friendly.getArmies() == 6, "Invalid Advance changed armies");

        std::cout << "3. Airlift: non-adjacent owned territories, positive available armies\n";
        Airlift airlift(&alice, &home, &airTarget, 2);
        check(airlift.validate(), "Non-adjacent Airlift rejected"); airlift.execute();
        check(home.getArmies() == 11 && airTarget.getArmies() == 4, "Airlift effect incorrect");
        Airlift enemyAirSource(&alice, &border, &airTarget, 1), enemyAirTarget(&alice, &home, &border, 1);
        Airlift sameAir(&alice, &home, &home, 1), noArmies(&alice, &home, &airTarget, 12);
        Airlift zeroAir(&alice, &home, &airTarget, 0), negativeAir(&alice, &home, &airTarget, -1);
        reject(enemyAirSource); reject(enemyAirTarget); reject(sameAir); reject(noArmies); reject(zeroAir); reject(negativeAir);
        check(home.getArmies() == 11 && airTarget.getArmies() == 4, "Invalid Airlift changed armies");

        std::cout << "4. Bomb: adjacent enemy only; half the armies remain (integer division)\n";
        Bomb bomb(&alice, &border);
        check(bomb.validate(), "Valid Bomb rejected"); bomb.execute();
        check(border.getArmies() == 4, "Bomb did not halve armies");
        border.setArmies(5); bomb.execute();
        check(border.getArmies() == 2, "Odd Bomb rounding incorrect");
        Bomb ownBomb(&alice, &home), remoteBomb(&alice, &remote);
        reject(ownBomb); reject(remoteBomb);
        check(home.getArmies() == 11 && remote.getArmies() == 0, "Invalid Bomb changed armies");

        std::cout << "5. Negotiate: bilateral Advance/Bomb restriction, friendly moves allowed, expiry\n";
        Negotiate self(&alice, &alice); reject(self);
        Negotiate negotiate(&alice, &bob);
        check(negotiate.validate(), "Valid Negotiate rejected"); negotiate.execute();
        Advance attack(&alice, &home, &border, 1), reverse(&bob, &border, &home, 1);
        Bomb reverseBomb(&bob, &home);
        reject(attack); reject(reverse); reject(bomb); reject(reverseBomb);
        Advance friendlyDuringPact(&alice, &home, &friendly, 1);
        check(friendlyDuringPact.validate(), "Negotiation blocked friendly movement");
        Orders::endTurn();
        check(attack.validate() && reverse.validate() && bomb.validate(), "Negotiation did not expire");

        std::cout << "6. Blockade: double armies and transfer both ownership records to real Neutral\n";
        Blockade blockade(&alice, &airTarget);
        check(blockade.validate(), "Valid Blockade rejected"); blockade.execute();
        Player* neutral = airTarget.getOwner();
        check(neutral && neutral != &alice && neutral->getName() == "Neutral" && airTarget.getArmies() == 8,
              "Blockade did not create Neutral ownership");
        check(contains(neutral->getTerritories(), &airTarget) && !contains(alice.getTerritories(), &airTarget),
              "Blockade ownership lists inconsistent");
        Blockade secondBlockade(&alice, &friendly); secondBlockade.execute();
        check(friendly.getOwner() == neutral && friendly.getArmies() == 12, "Neutral not reused");
        Blockade enemyBlockade(&alice, &border); reject(enemyBlockade);

        std::cout << "7. Captures update both Players; conquest reward eligibility is once per turn\n";
        // Empty garrisons give deterministic captures using the real combat implementation.
        Advance firstCapture(&alice, &home, &capture1, 2); firstCapture.execute();
        Advance secondCapture(&alice, &home, &capture2, 1); secondCapture.execute();
        check(capture1.getOwner() == &alice && capture1.getArmies() == 2 && capture2.getOwner() == &alice,
              "Conquest effect incorrect");
        check(!contains(bob.getTerritories(), &capture1) && contains(alice.getTerritories(), &capture1),
              "Conquest ownership lists inconsistent");
        check(Orders::getConquerors().size() == 1 && Orders::getConquerors()[0] == &alice,
              "More than one reward eligibility entry per player/turn");
        Orders::endTurn();
        check(Orders::getConquerors().empty(), "Conquest flags did not reset");
        bob.addTerritory(&capture2); capture2.setArmies(0);
        secondCapture.execute();
        check(Orders::getConquerors().size() == 1, "Next-turn conquest was not recorded");
        std::cout << "Card delivery itself requires the controller's shared Deck; it is not faked here.\n";

        std::cout << "8. Simultaneous combat: real random volley, nonnegative survivors\n";
        const int sourceBefore = home.getArmies(); border.setArmies(2);
        Advance combat(&alice, &home, &border, 2); combat.execute();
        check(home.getArmies() >= sourceBefore - 2 && home.getArmies() <= sourceBefore
              && border.getArmies() >= 0 && border.getArmies() <= 2, "Combat army bounds violated");
        if (border.getOwner() == &alice) check(home.getArmies() == sourceBefore - 2, "Captured armies also retreated");
        else check(border.getOwner() == &bob && border.getArmies() > 0, "Combat ownership incorrect");
        bob.addTerritory(&border); border.setArmies(100);
        const int beforeRepelled = home.getArmies();
        Advance repelled(&alice, &home, &border, 1); repelled.execute();
        check(border.getOwner() == &bob && border.getArmies() >= 99
              && home.getArmies() >= beforeRepelled - 1 && home.getArmies() <= beforeRepelled,
              "Failed attack or retreat arithmetic incorrect");
        border.setArmies(4);

        std::cout << "9. Null/overflow inputs are rejected, not dereferenced\n";
        Deploy nullIssuer(nullptr, &home, 1); reject(nullIssuer);
        Advance nullSource(&alice, nullptr, &border, 1); reject(nullSource);
        Bomb nullBomb(&alice, nullptr); reject(nullBomb);
        Blockade nullBlockade(&alice, nullptr); reject(nullBlockade);
        Airlift nullAirlift(&alice, &home, nullptr, 1); reject(nullAirlift);
        Negotiate nullNegotiation(&alice, nullptr); reject(nullNegotiation);
        alice.addReinforcements(3);
        const int saved = home.getArmies(); home.setArmies(std::numeric_limits<int>::max());
        Deploy overflow(&alice, &home, 1); reject(overflow); home.setArmies(saved);
        airTarget.setArmies(std::numeric_limits<int>::max());
        Blockade overflowBlockade(neutral, &airTarget); reject(overflowBlockade); airTarget.setArmies(8);

        std::cout << "10. Original list add/remove/move/copy/execute demonstration\n";
        OrdersList list;
        list.add(new Deploy(&alice, &home, 3));
        list.add(new Advance(&alice, &home, &capture1, 1));
        list.add(new Bomb(&alice, &border));
        list.add(new Blockade(&alice, &capture2));
        list.add(new Airlift(&alice, &home, &capture1, 1));
        list.add(new Negotiate(&alice, &bob));
        list.add(new Deploy(&alice, &home, 1));
        list.remove(6); list.move(1, 2);
        OrdersList copied(list), assigned; assigned = list;
        for (int i = 0; i < 6; ++i) check(copied.getOrder(i) != list.getOrder(i)
            && assigned.getOrder(i) != list.getOrder(i), "Order cloning shares owned allocations");
        static_assert(std::is_copy_assignable<Deploy>::value && std::is_copy_assignable<Advance>::value
            && std::is_copy_assignable<Bomb>::value && std::is_copy_assignable<Blockade>::value
            && std::is_copy_assignable<Airlift>::value && std::is_copy_assignable<Negotiate>::value,
            "Concrete orders must remain assignable");
        std::cout << list << *list.getOrder(0) << '\n';
        list.execute();
        check(list.getOrdersListSize() == 0 && alice.getReinforcementPool() == 0, "List execution/spending failed");
        check(capture2.getOwner() == neutral && border.getArmies() == 2, "Mixed order effects failed");

        // Real Player API/Part 3 constructor integration, without the broken Cards module.
        alice.setOrdersList(&list); alice.addReinforcements(2);
        alice.issueOrder("deploy"); alice.issueOrder("advance");
        check(list.getOrder(0)->getName() == "Deploy" && list.getOrder(1)->getName() == "Advance",
              "Player constructor integration failed");
        list.getOrder(0)->execute(); list.remove(0); list.remove(0);
        check(alice.getReinforcementPool() == 0, "Player-issued Deploy deducted incorrectly");
        Orders::resetGameState(); // Before territories/Players leave scope.
        check(!airTarget.getOwner() && !friendly.getOwner() && !capture2.getOwner(), "Neutral cleanup failed");
        std::cout << "PASS: all Orders driver checks; turn hooks/card delivery still need GameEngine integration.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
