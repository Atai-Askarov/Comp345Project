// demonstration driver of the cards component

#include "Cards.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "../Player/Player.h"
#include "../Map/Map.h"


// MAIN METHOD
int main() {
    srand(static_cast<unsigned int>(time(0))); // random seed for card draws

    Continent continent("Test", 0, "red");
    Territory home(1, "Home", &continent), other(2, "Other", &continent), enemy(3, "Enemy", &continent);
    Player player("Card player"), opponent("Opponent");
    player.addTerritory(&home); player.addTerritory(&other); opponent.addTerritory(&enemy);
    home.addNeighbour(&enemy);
    home.setArmies(5); other.setArmies(5); enemy.setArmies(8);


    // welcome message
    std::cout << "-----------------------------------------------------" << std::endl;
    std::cout << "WARZONE CARDS DEMONSTRATION" << std::endl;
    std::cout << "-----------------------------------------------------" << std::endl;


    std::cout << "\n1. Creating deck with 3 copies of every card type:" << std::endl;
    Deck* gameDeck = new Deck();
    CardType types[] = { // array of all card types
        CardType::Bomb, 
        CardType::Reinforcement, 
        CardType::Blockade, 
        CardType::Airlift, 
        CardType::Diplomacy};
    
        // add new card for each type, 3 copies each
    for (CardType type : types) {
        for (int copy = 0; copy < 3; ++copy) {
            gameDeck->addCard(new Card(type)); // adding card into deck through for loop
        }
    }
    std::cout << *gameDeck; // print deck content



    std::cout << "\n2. Creating an empty hand and empty orders list:" << std::endl;
    Hand* playerHand = new Hand(); //
    OrdersList* ordersList = new OrdersList(); // making new orders list
    player.setHand(playerHand);
    player.setOrdersList(ordersList);
    std::cout << *playerHand << std::endl; // print hand content



// drawing 5 cards into hand
    std::cout << "\n3. Drawing 5 random cards into the hand:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        gameDeck->draw(*playerHand); 
    }

    std::cout << "\n" << *playerHand; 
    std::cout << "Deck size after draws: " << gameDeck->getSize() << std::endl; 



    std::cout << "\n4. Playing every card in the hand safely:" << std::endl;
    const auto drawnCards = *playerHand->getCards();
    for (Card* cardToPlay : drawnCards) {
        if (!cardToPlay->play(*playerHand, *gameDeck, *ordersList, player))
            std::cout << "Kept " << cardToPlay->getTypeAsString() << ": no eligible target or defined reinforcement rule.\n";
    }




    std::cout << "\n5. Checking the result:" << std::endl;
    std::cout << *playerHand;
    std::cout << "Deck size after plays: " << gameDeck->getSize() << std::endl; // getting deck size after plays

    const bool verified = playerHand->getSize() + gameDeck->getSize() == 15
        && ordersList->getOrdersListSize() == static_cast<size_t>(5 - playerHand->getSize());
    if (verified) {
        std::cout << "CHECKED: all 15 cards accounted for; one order per played card." << std::endl;
    } else {
        std::cout << "CHECKED FAILED." << std::endl; // invalid hand is not empty 
    }



    // demonstrating copy constructor and assignment operator
    std::cout << "\n6. Copy constructor and assignment operator check:" << std::endl;
    
    for (int i = 0; i < 3; ++i) {
        gameDeck->draw(*playerHand);
    }

    // verifying copy constructor 
    Hand* copiedHand = new Hand(*playerHand); // copy it 
    std::cout << "Original hand Size: " << playerHand->getSize() << "\nCopied hand size: " << copiedHand->getSize() << std::endl;

    // verifying assignment operator
    Hand* assignedHand = new Hand();
    *assignedHand = *playerHand;
    std::cout << "Assigned hand size: " << assignedHand->getSize() << std::endl;



    // clean up memory
    player.setHand(nullptr);
    player.setOrdersList(nullptr);
    delete gameDeck;
    delete playerHand;
    delete copiedHand;
    delete assignedHand; 
    delete ordersList;



    // finished message
    std::cout << "\nDemonstration finished successfully." << std::endl;
    return verified ? 0 : 1;
}

