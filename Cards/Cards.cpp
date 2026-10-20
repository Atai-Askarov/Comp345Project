// including guards
#include "Cards.h"
#include <cstdlib>
#include <algorithm>

// order list placeholder implementation
void OrdersList::addOrder(const std::string& orderName) {
    std::cout << orderName << " order added to player's order list." << std::endl;
}



// CARD IMPLEMENTATION------------------------
// default constructor
Card::Card() {
    type = new CardType(CardType::Bomb); // default is bomb
}

// type constructor
Card::Card(CardType type) {
    this->type = new CardType(type);
}

// copy constructor
Card::Card(const Card& other) {
    this->type = new CardType(*other.type);
}

// destructor
Card::~Card() {
    delete type;
    type = nullptr; // nullptr to avoid pointer pointing to deallocated memory
}

// assignment operator
Card& Card::operator=(const Card& other) {
    if (this != &other) { // conditional to avoid self assignment
        delete type; 
        type = new CardType(*other.type); // deep copy of the type
    }
    return *this;
}

// getters
CardType Card::getType() const { 
    return *type; 
}


// getting the string version of the card type for printing 
std::string Card::getTypeAsString() const {
    switch (*type) {
        case CardType::Bomb:          
        return "Bomb";
       
        case CardType::Reinforcement: 
        return "Reinforcement";
       
        case CardType::Blockade:      
        return "Blockade";
      
        case CardType::Airlift:       
        return "Airlift";
      
        case CardType::Diplomacy:     
        return "Diplomacy";
      
        default: // default case                  
        return "Unknown";
    }
}


// printing the card type
std::ostream& operator<<(std::ostream& os, const Card& card) {
    os << card.getTypeAsString(); // get string ver
    return os;
}


// play function
void Card::play(Hand& hand, Deck& deck, OrdersList& ordersList) {
    std::cout << "Playing card: " << getTypeAsString() << std::endl; // shows current playing card
    ordersList.addOrder(getTypeAsString()); // add order to orders list
    
    // create a fresh deep copy to put back into the deck pool
    deck.addCard(new Card(*this));
    
    // delete card from player's hand and free the memory
    hand.removeCard(this); 
    delete this; 
}



// DECK IMPLEMENTATION------------------------
// default constructor
Deck::Deck() {
    cards = new std::vector<Card*>();
}

// copy constructor
Deck::Deck(const Deck& other) {
    cards = new std::vector<Card*>();
    for (Card* card : *(other.cards)) {
        cards->push_back(new Card(*card));  // deep copy of each card
    }
}


// destructor
Deck::~Deck() {
    if (cards) {
        for (Card* card : *cards) { // delete each card in the deck
            delete card;
        }
        delete cards;
        cards = nullptr;
    }
}

// assignment operator
Deck& Deck::operator=(const Deck& other) {
    if (this != &other) {
        for (Card* card : *cards) {
            delete card;
        }
        cards->clear(); // clear vector before copying new cards
        
        for (Card* card : *(other.cards)) {
            cards->push_back(new Card(*card)); // deep copy of each card
        }
    }
    return *this;
}


// adding card to deck
void Deck::addCard(Card* card) {
    cards->push_back(card);
}


// draw function to draw random card from deck to hnd
void Deck::draw(Hand& hand) {
    if (cards->empty()) {
        std::cout << "Deck is empty, cannot draw." << std::endl;
        return;
    }
    int randomIndex = rand() % cards->size(); // generate random index, use modulus to ensure index is within bounds
    Card* drawnCard = (*cards)[randomIndex]; // get the card of that specific index
    cards->erase(cards->begin() + randomIndex); // remove the card from deck
    hand.addCard(drawnCard);
}


// getter
int Deck::getSize() const { 
    return cards->size(); 
}


// printing deck content
std::ostream& operator<<(std::ostream& os, const Deck& deck) {
    os << "Deck Content (" << deck.getSize() << " cards):\n";
    for (Card* card : *(deck.cards)) { // iterate through the deck and print each card
        os << " - " << *card << "\n";
    }
    return os;
}



// HAND IMPLEMENTATION------------------------
// default constructor
Hand::Hand() {
    cards = new std::vector<Card*>();
}


// copy constructor
Hand::Hand(const Hand& other) {
    cards = new std::vector<Card*>();
    for (Card* card : *(other.cards)) {
        cards->push_back(new Card(*card)); 
    }
}


// destructor
Hand::~Hand() {
    if (cards) {
        for (Card* card : *cards) { // delete each card in the hand
            delete card;
        }
        delete cards;
        cards = nullptr;
    }
}

// assignment operator
Hand& Hand::operator=(const Hand& other) {
    if (this != &other) {
        for (Card* card : *cards) {
            delete card;
        }
        cards->clear();
        
        // deep copy of each card from the other hand
        for (Card* card : *(other.cards)) {
            cards->push_back(new Card(*card));
        }
    }
    return *this;
}


// adding card to hand
void Hand::addCard(Card* card) {
    cards->push_back(card);
}


// removing card from hand
void Hand::removeCard(Card* card) {
    auto it = std::find(cards->begin(), cards->end(), card);
    if (it != cards->end()) { // if card is found in the hand, remove it
        cards->erase(it);
    }
}


// getter for hand size
int Hand::getSize() const { 
    return cards->size(); 
}

// getter for collection of cards in hand
std::vector<Card*>* Hand::getCards() const { 
    return cards; 
}

// printing hand content
std::ostream& operator<<(std::ostream& os, const Hand& hand) {
    os << "Hand Content (" << hand.getSize() << " cards).\n";
    for (Card* card : *(hand.cards)) { // iterate through the hand and print each card
        os << " - " << *card << "\n";
    }
    return os;
}

