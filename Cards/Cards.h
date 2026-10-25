#ifndef CARDS_H // defining guards
#define CARDS_H

#include <iostream>
#include <string>
#include <vector>


// defining class
class Deck;
class Hand;

// Share the real OrdersList with Player.
#include "../Orders/Orders.h"

// card types
// enum class for type safety and avoiding conversions
enum class CardType { 
    Bomb, 
    Reinforcement, 
    Blockade, 
    Airlift, 
    Diplomacy 
};


// card class
class Card {
public:
    Card();
    Card(CardType type); // for type
    Card(const Card& other); // for copy
    ~Card(); // for destructor
    Card& operator=(const Card& other); // for assignment operator
    
    friend std::ostream& operator<<(std::ostream& os, const Card& card); // for printing thecard type

    void play(Hand& hand, Deck& deck, OrdersList& ordersList); // play function
    CardType getType() const;
    std::string getTypeAsString() const;

private:
    CardType* type; // pointer type 
};



// deck class
// drawing cards from deck to hand
class Deck {
public:
    Deck();
    Deck(const Deck& other);
    ~Deck();
    Deck& operator=(const Deck& other);
    
    friend std::ostream& operator<<(std::ostream& os, const Deck& deck);

    void draw(Hand& hand);
    void addCard(Card* card);
    int getSize() const;

private:
    std::vector<Card*>* cards; // Dynamic vector pointer to satisfy guideline #1
};




// hand class
// playing cards from hand to orders list
class Hand {
public:
    Hand();
    Hand(const Hand& other);
    ~Hand(); // for destructor
    Hand& operator=(const Hand& other);
    
    friend std::ostream& operator<<(std::ostream& os, const Hand& hand);

    void addCard(Card* card);
    void removeCard(Card* card);
    int getSize() const;
    std::vector<Card*>* getCards() const;

private:
    std::vector<Card*>* cards; // vector pointer, use vector as it automatically adjusts size since hand stores a variable number of cards that constantly changes during gameplay
};

#endif

