// including guards
#include "Cards.h"
#include "../Player/Player.h"
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <stdexcept>

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
bool Card::play(Hand& hand, Deck& deck, OrdersList& ordersList, Player& player) {
    if (player.getHand() != &hand || player.getOrdersList() != &ordersList
            || std::find(hand.getCards()->begin(), hand.getCards()->end(), this) == hand.getCards()->end()) {
        throw std::invalid_argument("Card must belong to the issuing player's attached hand.");
    }
    std::string orderType;
    switch (*type) {
        case CardType::Bomb: orderType = "bomb"; break;
        case CardType::Blockade: orderType = "blockade"; break;
        case CardType::Airlift: orderType = "airlift"; break;
        case CardType::Diplomacy: orderType = "negotiate"; break;
        case CardType::Reinforcement:
            // Neither supplied assignment defines this card's bonus/effect.
            // Keep it in the hand until the team specifies the rule.
            return false;
        default: throw std::invalid_argument("Unknown card type.");
    }
    const int index = static_cast<int>(ordersList.getOrdersListSize());
    try { player.issueOrder(orderType); }
    catch (const std::logic_error&) { return false; } // No target: keep the card.
    try { deck.addCard(this); }
    catch (...) { ordersList.remove(index); throw; }
    hand.removeCard(this); // Ownership transfers to Deck; no copying or delete this.
    std::cout << "Played " << getTypeAsString() << " card for " << player.getName() << '\n';
    return true;
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
    hand.addCard(drawnCard);
    cards->erase(cards->begin() + randomIndex); // Transfer only after insertion succeeds.
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

