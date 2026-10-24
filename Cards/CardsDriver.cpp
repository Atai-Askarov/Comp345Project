// demonstration driver of the cards component

#include "Cards.h"
#include <cstdlib>
#include <ctime>
#include <iostream>


// MAIN METHOD
int main() {
    srand(static_cast<unsigned int>(time(0))); // random seed for card draws


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
    std::cout << *playerHand << std::endl; // print hand content



// drawing 5 cards into hand
    std::cout << "\n3. Drawing 5 random cards into the hand:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        gameDeck->draw(*playerHand); 
    }

    std::cout << "\n" << *playerHand; 
    std::cout << "Deck size after draws: " << gameDeck->getSize() << std::endl; 



    std::cout << "\n4. Playing every card in the hand safely:" << std::endl;
    while (playerHand->getSize() > 0) {
        Card* cardToPlay = playerHand->getCards()->front(); 
        // play() will internally remove it from hand, add a new copy to deck, and free memory safely
        cardToPlay->play(*playerHand, *gameDeck, *ordersList); 
    }




    std::cout << "\n5. Checking the result:" << std::endl;
    std::cout << *playerHand;
    std::cout << "Deck size after plays: " << gameDeck->getSize() << std::endl; // getting deck size after plays

    if (playerHand->getSize() == 0 && gameDeck->getSize() == 15) { // conditional to check
        std::cout << "CHECKED: hand is empty deck has 15 cards." << std::endl; 
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
    delete gameDeck;
    delete playerHand;
    delete copiedHand;
    delete assignedHand; 
    delete ordersList;



    // finished message
    std::cout << "\nDemonstration finished successfully." << std::endl;
    return 0;
}

