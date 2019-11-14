//
//  GameManager.cpp
//  Texas Hold 'em
//
//  Created by Jonathan Redwine on 10/9/19.
//  Copyright © 2019 JonathanRedwine. All rights reserved.
//

#include <random>
#include "AI.cpp"
using namespace std;



// GameManager Class
// This class handles facilitating the game of Texas Hold 'em.
// For example, this class holds the deck of Card objects
// and deals cards at the appropriate time in the game.
// The class also facilitates betting rounds between the user and the AI,
// and keeps track of bet sizes, user/AI stack sizes, pot size, etc.
// The class also determines the winner of hands, analyzing the cards in user's and AI's hands
// as well as the board cards to determine who has the best hand.
class GameManager {
public:
    Card* deck = new Card[53]; // 53rd card is a "dead" card
    Card* drawnCards = new Card[9]; // 9 cards will be drawn per hand - 2 per player, 5 board cards
    int potSize;
    int userStack;
    int AIStack;
    Card* userHand = new Card[2];
    Card* AIHand = new Card[2];
    GameManager() {
        initDeck();
    }
    void initDeck();
    Card drawCard();
    void shuffleDeck();
    void finishHand(int handWinner, int hand);
    void displayTable();
    int bettingRound(AI ai, int firstBettor, int bettingRound);
    int userBet(int currBet, int userLastBet);
    int findBestHand(Card* cards);
    int findStraightFlush(Card* cards);
    int findFourOfAKind(Card* cards);
    int findFullHouse(Card* cards);
    int findFlush(Card* cards);
    int findStraight(Card* cards);
    int findThreeOfAKind(Card* cards);
    int findTwoPair(Card* cards);
    int findPair(Card* cards);
    int resolveTie(int handStrength, Card* userCards, Card* AICards);
    int resolveTieStraightFlush(Card* userCards, Card* AICards);
    int resolveTieFourOfAKind(Card* userCards, Card* AICards);
    int resolveTieFullHouse(Card* userCards, Card* AICards);
    int resolveTieFlush(Card* userCards, Card* AICards);
    int resolveTieStraight(Card* userCards, Card* AICards);
    int resolveTieThreeOfAKind(Card* userCards, Card* AICards);
    int resolveTieTwoPair(Card* userCards, Card* AICards);
    int resolveTiePair(Card* userCards, Card* AICards);
    int resolveTieHighCard(Card* userCards, Card* AICards);
};


// Randomly draw a card (that hasn't already been drawn) from the deck of Card objects
// Returns the drawn card
Card GameManager::drawCard() {
    int keepTrying = 1;
    Card newCard;
    Card alreadyDrawnCard;
    while (keepTrying) { // loop until we find a new, unique card
        keepTrying = 0; // at beginning of loop, assume new card is unique
        newCard = deck[rand()%52]; // try new card
        for (int i = 0; i < 9; i++) { // look through array of already drawn cards
            alreadyDrawnCard = drawnCards[i];
            if ((alreadyDrawnCard.value == newCard.value) && (alreadyDrawnCard.suit == newCard.suit)) {
                keepTrying = 1; // if our new card matches an already drawn card, we must redraw
            }
            if ((alreadyDrawnCard.value == -1) && (keepTrying == 0)) { // if we reach "dead" cards with keepTrying = 0, then our newCard is unique
                drawnCards[i] = newCard; // put new card in drawnCards array
                return newCard;
            }
        }
    }
    return deck[52]; // return "dead" card, but this actually should never happen
}


// Shuffles the deck, resetting a hand
void GameManager::shuffleDeck() { // essentially just clears the already drawn cards array, filling it with "dead" cards
    for (int i = 0; i < 9; i++) {
        drawnCards[i] = deck[52]; // deck[52] is a placeholder "dead" card
    }
}

// Finish the hand, putting appropriate chips in winner's stacks
void GameManager::finishHand(int handWinner, int hand) {
    if (handWinner == 1) { // if user won the hand
        userStack += potSize; // award user pot
        cout << "You win the pot of $" << potSize << "." << endl;
    } else if (handWinner == 2) { // if AI won the hand
        AIStack += potSize; // award AI pot
        cout << "Daniel wins the pot of $" << potSize << "." << endl;
    } else if (handWinner == 0) { // if user and AI tied
        int userWinnings = 0, AIWinnings = 0;
        userWinnings = potSize / 2;
        AIWinnings = potSize / 2; // chop the pot
        if ((potSize % 2) == 1) { // if pot is odd number, give extra dollar to dealer
            if ((hand)==0) {
                AIWinnings += 1;
            } else {
                userWinnings += 1;
            }
        }
        userStack += userWinnings;
        AIStack += AIWinnings;
        cout << "You receive $" << userWinnings << "." << endl;
        cout << "Daniel receives $" << AIWinnings << "." << endl;
    }
}


// Function for hosting a betting round
// Parameters are the AI, which bettor is first to bet, and the betRound that is happening
// bettor: 0 means user bets first, 1 means AI bets first
// betRound: 0 is pre-flop, 1 is flop, 2 is turn, and 3 is river
// Returns an int: 1 means no fold happened, and the hand should proceed. -1 means a fold happened, hand ends.
int GameManager::bettingRound(AI ai, int bettor, int betRound) {
    int currBet = 0, keepGoing = 1, userLastBet = 0, AILastBet = 0, userHadAction = 0, AIHadAction = 0;
    if (betRound == 0) { // if pre-flop, set big/little blinds as current bets
        if ((bettor%2) == 0) { // AI is dealer
            AILastBet = 2; // account for blinds
            userLastBet = 1;
        } else { // user is dealer
            AILastBet = 1; // account for blinds
            userLastBet = 2;
        }
        currBet = 2;
    }
    while (keepGoing) {
        if ((bettor%2) == 0) { // AI is dealer, user bets first
            int thisBet = userBet(currBet, userLastBet);
            if (thisBet != -1) { // if the user didn't choose to fold
                userStack -= thisBet;
                potSize += thisBet;
                userLastBet = userLastBet + thisBet;
                currBet = userLastBet;
            } else { // if the user chose to fold
                userLastBet = -1;
            }
            userHadAction = 1;
        }
        else { // User is dealer, AI bets first
            Card* boardCards = new Card[5];
            for (int i = 0; i < 5; i++) {
                boardCards[i] = drawnCards[i+4];
            }
            int thisAIBet = ai.makeBetDecision(currBet, AILastBet, potSize, AIStack, userStack, AIHand, boardCards, betRound, deck);
            if (thisAIBet != -1) { // if AI didn't choose to fold
                AIStack -= thisAIBet;
                potSize += thisAIBet;
                AILastBet += thisAIBet;
                currBet = AILastBet;
            } else { // if the AI chose to fold
                AILastBet = -1;
            }
            AIHadAction = 1;
        }
        bettor += 1;
        if (userLastBet == -1) {
            finishHand(2, bettor);
            return -1; // if user folds, finish hand with AI as winner
        } else if (AILastBet == -1) {
            finishHand(1, bettor);
            return -1; // if AI folds, finish hand with user as winner
        }
        else if ((userLastBet == AILastBet) && (userHadAction == 1) && (AIHadAction == 1)) { // end hand if same bets and both had action
            keepGoing = 0;
        }
        cout << "Pot: $" << potSize << endl;
    }
    return 1;
}


// Function for retrieving the user's bet
// Parameters are:
// currBet - the total amount bet at this round of the game
// userLastBet - the amount the user has already put in this round
// Thus, the user would owe (currBet - userLastBet) if they wish to call
// Returns an int:
// -1 - user folded
// Any other int - the amount the user is putting in the bet (whether it's a call or a raise)
int GameManager::userBet(int currBet, int userLastBet) {
    int bet = 0;
    int amountOwed = currBet - userLastBet;
    while (true) {
        if (currBet == 0) { // if user first to act, or AI checks into user
            cout << "How much would you like to bet? You can bet anywhere from $2 to $" << userStack << ".  Enter 0 to check." << endl;
            cin >> bet;
            if (bet == 0) {
                cout << "The player checks." << endl;
                return 0;
            } else if ((bet >= 2) && (bet <= userStack)) {
                cout << "The player bets $" << bet << "." << endl;
                return bet;
            } else {
                cout << "Please enter a valid bet." << endl;
            }
        } else { // if AI bets into user (also if AI at small blind calls big blind)
            if (amountOwed != 0) {
                cout << "The action is $" << amountOwed << " to call." << endl;
            }
            cout << "How much would you like to bet?" << endl;
            if (amountOwed != 0) {
                cout << "0 - Fold." << endl;
            }
            if (amountOwed >= userStack) {
                cout << "1 - Go all in, putting in $" << userStack << "." << endl;
                cin >> bet;
                if (bet == 0) {
                    cout << "The player folds." << endl;
                    return -1;
                } else if (bet == 1) {
                    cout << "The player goes all in for $" << userStack << "!" << endl;
                    return userStack;
                } else {
                    cout << "Please enter a valid bet." << endl;
                }
            } else {
                if (amountOwed == 0) {
                    cout << "1 - Check." << endl;
                } else {
                    cout << "1 - Call, putting in $" << amountOwed << "." << endl;
                }
                if (2*currBet >= userStack) {
                    cout << "To go all in, enter the value of your remaining chips, $" << userStack << endl;
                    cin >> bet;
                    if (bet == 0) {
                        cout << "The player folds." << endl;
                        return -1;
                    } else if (bet == 1) {
                        cout << "The player calls the bet of $" << amountOwed << "." << endl;
                        return amountOwed;
                    } else if (bet == userStack) {
                        cout << "The player goes all in for $" << userStack << "!" << endl;
                        return userStack;
                    } else {
                        cout << "Please enter a valid bet." << endl;
                    }
                } else {
                    cout << "To raise, enter the amount you'd like to bet.  You may raise anywhere from $" << (2*currBet)-userLastBet << " to $" << userStack << "." << endl;
                    cin >> bet;
                    if (bet == 0) {
                        cout << "The player folds." << endl;
                        return -1;
                    } else if (bet == 1) {
                        cout << "The player calls the bet of $" << amountOwed << "." << endl;
                        return amountOwed;
                    } else if ((2*currBet)-userLastBet && (bet <= userStack)) {
                        cout << "The player bets $" << bet << "." << endl;
                        return bet;
                    } else {
                        cout << "Please enter a valid bet." << endl;
                    }
                }
            }
        }
    }
    return bet;
}


// Function for displaying the state of the table to the console
// Shows current board cards, user and AI stacks, user's cards, etc.
void GameManager::displayTable() {
    /*
    cout << endl << "Daniel's hand: ";  // DELETE WHEN AI HAND SHOULD BE HIDDEN
    for (int i = 0; i < 2; i++) {
        if (AIHand[i].value == 10) {
            cout << "T";
        } else if (AIHand[i].value == 11) {
            cout << "J";
        } else if (AIHand[i].value == 12) {
            cout << "Q";
        } else if (AIHand[i].value == 13) {
            cout << "K";
        } else if (AIHand[i].value == 14) {
            cout << "A";
        } else {
            cout << AIHand[i].value;
        }
        cout << AIHand[i].suit << " ";
    }
     */
    cout << endl << "Daniel's stack: $" << AIStack << endl;
    cout << endl << endl << "Board: " << endl;;
    int numCards = 0;
    for (int i = 4; i < 9; i++) {
        if (drawnCards[i].value != -1) {
            numCards += 1;
        }
    }
    cout << " ";
    for (int i = 4; i < numCards + 4; i++) {
        cout << "--------    ";
    }
    cout << endl;
    for (int i = 4; i < numCards + 4; i++) {
        cout << "|        |  ";
    }
    cout << endl;
    for (int i = 4; i < numCards + 4; i++) {
        cout << "|        |  ";
    }
    cout << endl;
    for (int i = 4; i < numCards + 4; i++) {
        cout << "|";
        if (drawnCards[i].value == 2) {
            cout << "  Two   ";
        } else if (drawnCards[i].value == 3) {
            cout << " Three  ";
        } else if (drawnCards[i].value == 4) {
            cout << "  Four  ";
        } else if (drawnCards[i].value == 5) {
            cout << "  Five  ";
        } else if (drawnCards[i].value == 6) {
            cout << "  Six   ";
        } else if (drawnCards[i].value == 7) {
            cout << " Seven  ";
        } else if (drawnCards[i].value == 8) {
            cout << " Eight  ";
        } else if (drawnCards[i].value == 9) {
            cout << "  Nine  ";
        } else if (drawnCards[i].value == 10) {
            cout << "  Ten   ";
        } else if (drawnCards[i].value == 11) {
            cout << "  Jack  ";
        } else if (drawnCards[i].value == 12) {
            cout << " Queen  ";
        } else if (drawnCards[i].value == 13) {
            cout << "  King  ";
        } else {
            cout << "  Ace   ";
        }
        cout << "|  ";
    }
    cout << endl;
    for (int i = 4; i < numCards + 4; i++) {
        cout << "|";
        if (drawnCards[i].suit == 'H') {
            cout << " Hearts ";
        } else if (drawnCards[i].suit == 'D') {
            cout << "Diamonds";
        } else if (drawnCards[i].suit == 'S') {
            cout << " Spades ";
        } else {
            cout << " Clubs  ";
        }
        cout << "|  ";
    }
    cout << endl;
    for (int i = 4; i < numCards + 4; i++) {
        cout << "|        |  ";
    }
    cout << endl;
    for (int i = 4; i < numCards + 4; i++) {
        cout << "|        |  ";
    }
    cout << endl << " ";
    for (int i = 4; i < numCards + 4; i++) {
        cout << "--------    ";
    }
    cout << endl;
    cout << endl << "Pot: $" << potSize << endl << endl << "Your stack: $" << userStack << endl << "Your hand: ";
    for (int i = 0; i < 2; i++) {
        if (userHand[i].value == 10) {
            cout << "10";
        } else if (userHand[i].value == 11) {
            cout << "Jack";
        } else if (userHand[i].value == 12) {
            cout << "Queen";
        } else if (userHand[i].value == 13) {
            cout << "King";
        } else if (userHand[i].value == 14) {
            cout << "Ace";
        } else {
            cout << userHand[i].value;
        }
        cout << " of ";
        if (userHand[i].suit == 'H') {
            cout << "Hearts";
        } else if (userHand[i].suit == 'D') {
            cout << "Diamonds";
        } else if (userHand[i].suit == 'S') {
            cout << "Spades";
        } else {
            cout << "Clubs";
        }
        if (i == 0) {
            cout << ", ";
        }
    }
    cout << endl << endl;;
}





// ------  HAND FINDERS ------


// 8 - Straight flush
// 7 - Four of a kind
// 6 - Full house
// 5 - Flush
// 4 - Straight
// 3 - Three of a kind
// 2 - Two pair
// 1 - Pair
// 0 - High card

// Function for finding the best hand given seven cards (two hole cards and five board cards)
// Sequentially looks for hands in descending order of strength (straight flush, then four of a kind, etc)
// Returns an int referring to the best hand that exists in the seven input cards
// according to the above values
int GameManager::findBestHand(Card* cards) {
    int hand = -1;
    if ((hand = findStraightFlush(cards)) == 1){
        return 8;
    } else if ((hand = findFourOfAKind(cards)) == 1) {
        return 7;
    } else if ((hand = findFullHouse(cards)) == 1) {
        return 6;
    } else if ((hand = findFlush(cards)) == 1) {
        return 5;
    } else if ((hand = findStraight(cards)) == 1) {
        return 4;
    } else if ((hand = findThreeOfAKind(cards)) == 1) {
        return 3;
    } else if ((hand = findTwoPair(cards)) == 1) {
        return 2;
    } else if ((hand = findPair(cards)) == 1) {
        return 1;
    } else {
        cout << "High card." << endl;
        return 0;
    }
}

// Function for finding if a straight flush exists in a set of seven cards.
// Returns 1 if one exists, returns -1 if not
int GameManager::findStraightFlush(Card* cards) {
    int highest = 0;
    int highIndex = -1;
    char highSuit = 'x';
    for (int h = 0; h < 7; h++) {
        highest = cards[h].value;
        highIndex = h;
        highSuit = cards[h].suit;
        if (highest >= 5) { // the lowest straight has a high card of 5
            for (int i = 0; i < 7; i++) {
                if ((cards[i].value == highest - 1) && (cards[i].suit == highSuit)) { // if second high card in straight flush exists
                    for (int j = 0; j < 7; j++) {
                        if ((cards[j].value == highest - 2) && (cards[j].suit == highSuit)) { // if third high card in straight flush exists
                            for (int k = 0; k < 7; k++) {
                                if ((cards[k].value == highest - 3) && (cards[k].suit == highSuit)) { // if fourth high card in straight flush exists
                                    for (int l = 0; l < 7; l++) {
                                        if ((cards[l].value == highest - 4) && (cards[l].suit == highSuit)) { // if fifth high card in straight flush exists
                                            cout << "Straight flush! ";
                                            if (highest < 11) {
                                                cout << highest;
                                            } else if (highest == 11) {
                                                cout << "Jack";
                                            } else if (highest == 12) {
                                                cout << "Queen";
                                            } else if (highest == 13) {
                                                cout << "King";
                                            } else {
                                                cout << "Ace";
                                            }
                                            cout << " high." << endl;
                                            return 1;
                                        } else if ((highest == 5) && (cards[l].value == 14) && (cards[l].suit == highSuit)) {
                                            cout << "Straight flush! ";
                                            if (highest < 11) {
                                                cout << highest;
                                            } else if (highest == 11) {
                                                cout << "Jack";
                                            } else if (highest == 12) {
                                                cout << "Queen";
                                            } else if (highest == 13) {
                                                cout << "King";
                                            } else {
                                                cout << "Ace";
                                            }
                                            cout << " high." << endl;
                                            return 1; // if the straight is ace-5, must check if bottom card is ace (value = 14)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return -1;
}


// Function for finding if a four of a kind exists in a set of seven cards.
// Returns 1 if one exists, returns -1 if not
int GameManager::findFourOfAKind(Card* cards) {
    int highest = 0;
    int prevHighest = 15; // set prev highst above possible values
    int highIndex = -1;
    while (highest != -1) {
        for (int i = 0; i < 7; i++) {
            if ((cards[i].value > highest) && (cards[i].value < prevHighest)) {
                highest = cards[i].value; // find highest value in cards less than previous highest
                highIndex = i;
            }
        }
        if (highest == 0) {
            return -1; // break the loop if we couldn't find a new high card -> that means no four of a kind was found
        }
        int numMatches = 0;
        for (int i = 0; i < 7; i++) {
            if ((cards[i].value == highest) && (i != highIndex)) {
                numMatches += 1; // try to find cards that match this card
            }
        }
        if (numMatches == 3) { // if three matches exist, we have a four of a kind
            cout << "Four of a kind! ";
            if (highest < 11) {
                cout << highest;
            } else if (highest == 11) {
                cout << "Jack";
            } else if (highest == 12) {
                cout << "Queen";
            } else if (highest == 13) {
                cout << "King";
            } else {
                cout << "Ace";
            }
            cout << "s." << endl;
            return 1;
        }
        prevHighest = highest;
        highest = 0; // reset search variables
        highIndex = -1;
    }
    return -1;
}


// Function for finding if a full house exists in a set of seven cards.
// Returns 1 if one exists, returns -1 if not
int GameManager::findFullHouse(Card* cards) {
    int highest = 0;
    int prevHighest = 15; // set prev highst above possible values
    int highIndex = -1;
    while (highest != -1) {
        for (int i = 0; i < 7; i++) {
            if ((cards[i].value > highest) && (cards[i].value < prevHighest)) {
                highest = cards[i].value; // find highest value in cards less than previous highest
                highIndex = i;
            }
        }
        if (highest == 0) {
            return -1; // break the loop if we couldn't find a new high card -> that means no full house was found
        }
        int numMatches = 0;
        for (int i = 0; i < 7; i++) {
            if ((cards[i].value == highest) && (i != highIndex)) {
                numMatches += 1; // try to find two other cards that match this card
            }
        }
        if (numMatches == 2) { // if we find that there is a three of a kind, now we look for a different pair
            int three = highest; // remember the value that has three of a kind
            highest = 0;
            prevHighest = 15; // reset search variables
            highIndex = -1;
            while (highest != -1) {
                for (int i = 0; i < 7; i++) {
                    if ((cards[i].value > highest) && (cards[i].value < prevHighest) && (cards[i].value != three)) {
                        highest = cards[i].value; // find highest value in cards less than previous highest
                        highIndex = i;
                    }
                }
                if (highest == 0) { // if no pair was found in addition to the three of a kind, then no full house exists
                    return -1;
                }
                for (int i = 0; i < 7; i++) {
                    if ((cards[i].value == highest) && (i != highIndex)) { // if we find another pair
                        cout << "Full house! ";
                        if (three < 11) {
                            cout << three;
                        } else if (three == 11) {
                            cout << "Jack";
                        } else if (three == 12) {
                            cout << "Queen";
                        } else if (three == 13) {
                            cout << "King";
                        } else {
                            cout << "Ace";
                        }
                        cout << "s over ";
                        if (highest < 11) {
                            cout << highest;
                        } else if (highest == 11) {
                            cout << "Jack";
                        } else if (highest == 12) {
                            cout << "Queen";
                        } else if (highest == 13) {
                            cout << "King";
                        } else {
                            cout << "Ace";
                        }
                        cout << "s." << endl;
                        return 1;
                    }
                }
                prevHighest = highest;
                highest = 0; // reset search variables
                highIndex = -1;
            }
        }
        prevHighest = highest;
        highest = 0; // reset search variables
        highIndex = -1;
    }
    return -1;
}


// Function for finding if a flush exists in a set of seven cards.
// Returns 1 if one exists, returns -1 if not
int GameManager::findFlush(Card* cards) {
    int numHearts = 0, numDiamonds = 0, numSpades = 0, numClubs = 0;
    int highHeart = 0, highDiamond = 0, highSpade = 0, highClub = 0;
    for (int i = 0; i < 7; i++) {
        if (cards[i].suit == 'H') {
            numHearts += 1;
            if (cards[i].value > highHeart) {
                highHeart = cards[i].value;
            }
        } else if (cards[i].suit == 'D') {
            numDiamonds += 1;
            if (cards[i].value > highDiamond) {
                highDiamond = cards[i].value;
            }
        } else if (cards[i].suit == 'S') {
            numSpades += 1;
            if (cards[i].value > highSpade) {
                highSpade = cards[i].value;
            }
        } else {
            numClubs += 1;
            if (cards[i].value > highClub) {
                highClub = cards[i].value;
            }
        }
    }
    if (numHearts > 4) {
        cout << "Flush! Hearts, ";
        if (highHeart < 11) {
            cout << highHeart;
        } else if (highHeart == 11) {
            cout << "Jack";
        } else if (highHeart == 12) {
            cout << "Queen";
        } else if (highHeart == 13) {
            cout << "King";
        } else {
            cout << "Ace";
        }
        cout << " high." << endl;
        return 1;
    } else if (numDiamonds > 4) {
        cout << "Flush! Diamonds, ";
        if (highDiamond < 11) {
            cout << highDiamond;
        } else if (highDiamond == 11) {
            cout << "Jack";
        } else if (highDiamond == 12) {
            cout << "Queen";
        } else if (highDiamond == 13) {
            cout << "King";
        } else {
            cout << "Ace";
        }
        cout << " high." << endl;
        return 1;
    } else if (numSpades > 4) {
        cout << "Flush! Spades, ";
        if (highSpade < 11) {
            cout << highSpade;
        } else if (highSpade == 11) {
            cout << "Jack";
        } else if (highSpade == 12) {
            cout << "Queen";
        } else if (highSpade == 13) {
            cout << "King";
        } else {
            cout << "Ace";
        }
        cout << " high." << endl;
        return 1;
    } else if (numClubs > 4) {
        cout << "Flush! Clubs, ";
        if (highClub < 11) {
            cout << highClub;
        } else if (highClub == 11) {
            cout << "Jack";
        } else if (highClub == 12) {
            cout << "Queen";
        } else if (highClub == 13) {
            cout << "King";
        } else {
            cout << "Ace";
        }
        cout << " high." << endl;
        return 1;
    } else {
        return -1;
    }
    return -1;
}


// Function for finding if a straight exists in a set of seven cards.
// Returns 1 if one exists, returns -1 if not
int GameManager::findStraight(Card* cards) {
    int highest = 0;
    int prevHighest = 15; // set prev highest above possible values
    int highIndex = -1;
    while (highest != -1) {
        for (int i = 0; i < 7; i++) {
            if ((cards[i].value > highest) && (cards[i].value < prevHighest)) {
                highest = cards[i].value; // find highest value in cards less than previous highest
                highIndex = i;
            }
        }
        if (highest == 0) {
            return -1; // break the loop if we couldn't find a new high card -> that means no straight was found
        }
        if (highest >= 5) { // the lowest straight has a high card of 5
            for (int i = 0; i < 7; i++) {
                if (cards[i].value == highest - 1) { // if second high card in straight exists
                    for (int j = 0; j < 7; j++) {
                        if (cards[j].value == highest - 2) { // if third high card in straight exists
                            for (int k = 0; k < 7; k++) {
                                if (cards[k].value == highest - 3) { // if fourth high card in straight exists
                                    for (int l = 0; l < 7; l++) {
                                        if (cards[l].value == highest - 4) { // if fifth high card in straight exists
                                            cout << "Straight! ";
                                            if (highest < 11) {
                                                cout << highest;
                                            } else if (highest == 11) {
                                                cout << "Jack";
                                            } else if (highest == 12) {
                                                cout << "Queen";
                                            } else if (highest == 13) {
                                                cout << "King";
                                            } else {
                                                cout << "Ace";
                                            }
                                            cout << " high." << endl;
                                            return 1;
                                        } else if ((highest == 5) && (cards[l].value == 14)) {
                                            cout << "Straight! ";
                                            if (highest < 11) {
                                                cout << highest;
                                            } else if (highest == 11) {
                                                cout << "Jack";
                                            } else if (highest == 12) {
                                                cout << "Queen";
                                            } else if (highest == 13) {
                                                cout << "King";
                                            } else {
                                                cout << "Ace";
                                            }
                                            cout << " high." << endl;
                                            return 1; // if the straight is ace-5, must check if bottom card is ace (value = 14)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        prevHighest = highest;
        highest = 0; // otherwise, reset variables and try again
        highIndex = -1;
    }
    return -1;
}


// Function for finding if a three of a kind exists in a set of seven cards.
// Returns 1 if one exists, returns -1 if not
int GameManager::findThreeOfAKind(Card* cards) {
    int highest = 0;
    int prevHighest = 15; // set prev highest above possible values
    int highIndex = -1;
    while (highest != -1) {
        for (int i = 0; i < 7; i++) {
            if ((cards[i].value > highest) && (cards[i].value < prevHighest)) {
                highest = cards[i].value; // find highest value in cards less than previous highest
                highIndex = i;
            }
        }
        if (highest == 0) {
            return -1; // break the loop if we couldn't find a new high card -> that means no three of a kind was found
        }
        int n = 0;
        for (int i = 0; i < 7; i++) { // search if two other cards have same value (ie, three of a kind exists)
            if ((cards[i].value == highest) && (i != highIndex)) {
                n += 1; // tally number of matching cards
            }
        }
        if (n == 2) { // if two matches found
            cout << "Three of a kind! ";
            if (highest < 11) {
                cout << highest;
            } else if (highest == 11) {
                cout << "Jack";
            } else if (highest == 12) {
                cout << "Queen";
            } else if (highest == 13) {
                cout << "King";
            } else {
                cout << "Ace";
            }
            cout << "s." << endl;
            return 1;
        }
        prevHighest = highest;
        highest = 0; // otherwise, reset variables and try again
        highIndex = -1;
    }
    return -1;
}


// Function for finding if a two pair exists in a set of seven cards.
// Returns 1 if one exists, returns -1 if not
int GameManager::findTwoPair(Card* cards) {
    int highest = 0;
    int prevHighest = 15; // set prev highest above possible values
    int highIndex = -1;
    while (highest != -1) {
        for (int i = 0; i < 7; i++) {
            if ((cards[i].value > highest) && (cards[i].value < prevHighest)) {
                highest = cards[i].value; // find highest value in cards less than previous highest
                highIndex = i;
            }
        }
        if (highest == 0) {
            return -1; // break the loop if we couldn't find a new high card -> that means no two pair was found
        }
        for (int i = 0; i < 7; i++) {// find if a pair exists for this card
            if ((cards[i].value == highest) && (i != highIndex)) { // if one is found
                int secondHighest = 0;
                int prevSecondHighest = 15; // set up variable to search for second pair
                int secondHighIndex = -1;
                while (secondHighest != -1) {
                    for (int j = 0; j < 7; j++) { // find next highest card
                        if ((cards[j].value > secondHighest) && (cards[j].value < highest) && (cards[j].value < prevSecondHighest)) {
                            secondHighest = cards[j].value; // get next highest card not already looked at
                            secondHighIndex = j;
                        }
                    }
                    for (int j = 0; j < 7; j++) { // find if a pair exists for this second card
                        if ((cards[j].value == secondHighest) && (j != secondHighIndex)) {
                            cout << "Two pair! ";
                            if (highest < 11) {
                                cout << highest;
                            } else if (highest == 11) {
                                cout << "Jack";
                            } else if (highest == 12) {
                                cout << "Queen";
                            } else if (highest == 13) {
                                cout << "King";
                            } else {
                                cout << "Ace";
                            }
                            cout << "s and ";
                            if (secondHighest < 11) {
                                cout << secondHighest;
                            } else if (secondHighest == 11) {
                                cout << "Jack";
                            } else if (secondHighest == 12) {
                                cout << "Queen";
                            } else if (secondHighest == 13) {
                                cout << "King";
                            } else {
                                cout << "Ace";
                            }
                            cout << "s." << endl;
                            return 1;
                        }
                    }
                    if (secondHighest == 0) { // break if no second high card could be found (second pair does not exist)
                        secondHighest = -1;
                    } else {
                        prevSecondHighest = secondHighest;
                        secondHighest = 0; // otherwise, reset variables and keep looking for second pair
                        secondHighIndex = -1;
                    }
                }
            }
        }
        prevHighest = highest;
        highest = 0; // keep looking for first pair
        highIndex = -1;
    }
    return -1;
}


// Function for finding if a pair exists in a set of seven cards.
// Returns 1 if one exists, returns -1 if not
int GameManager::findPair(Card* cards) {
    int highest = 0;
    int prevHighest = 15; // set prev highest above possible values
    int highIndex = -1;
    while (highest != -1) {
        for (int i = 0; i < 7; i++) {
            if ((cards[i].value > highest) && (cards[i].value < prevHighest)) {
                highest = cards[i].value; // find highest value in cards less than previous highest
                highIndex = i;
            }
        }
        if (highest == 0) {
            return -1; // break the loop if we couldn't find a new high card -> that means we reached the end without finding a pair
        }
        for (int i = 0; i < 7; i++) { // find if a pair exists for this card
            if ((cards[i].value == highest) && (i != highIndex)) {
                cout << "Pair of ";
                if (highest < 11) {
                    cout << highest;
                } else if (highest == 11) {
                    cout << "Jack";
                } else if (highest == 12) {
                    cout << "Queen";
                } else if (highest == 13) {
                    cout << "King";
                } else {
                    cout << "Ace";
                }
                cout << "s." << endl;
                return 1; // return 1 if a pair is found
            }
        }
        prevHighest = highest;
        highest = 0; // otherwise, reset variables and try again
        highIndex = -1;
    }
    return -1;
}





// ------  TIE RESOLVERS ------

// If it's determined that the user and the AI have the same strength of hand (e.g., both have two pair)
// then the tie must be resolved.
// The way that the tie is resolved is dependent on the hand that the two players have
// For example, if both have two pair, first check whose high pair is higher,
// if same then check whose low pair is higher, if same then check whose fifth card is higher
// This function returns 1 if the user wins, 2 if the AI wins, and -1 if it's actually still a tie
int GameManager::resolveTie(int handStrength, Card* userCards, Card* AICards) {
    if (handStrength == 8) {
        return resolveTieStraightFlush(userCards, AICards);
    } else if (handStrength == 7) {
        return resolveTieFourOfAKind(userCards, AICards);
    } else if (handStrength == 6) {
        return resolveTieFullHouse(userCards, AICards);
    } else if (handStrength == 5) {
        return resolveTieFlush(userCards, AICards);
    } else if (handStrength == 4) {
        return resolveTieStraight(userCards, AICards);
    } else if (handStrength == 3) {
        return resolveTieThreeOfAKind(userCards, AICards);
    } else if (handStrength == 2) {
        return resolveTieTwoPair(userCards, AICards);
    } else if (handStrength == 1) {
        return resolveTiePair(userCards, AICards);
    } else if (handStrength == 0) {
        return resolveTieHighCard(userCards, AICards);
    } else {
        return -1;
    }
}


// Function for resolving a tie between two straight flushes
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTieStraightFlush(Card* userCards, Card* AICards) {
    int userStraight = 0, AIStraight = 0, userHigh = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15, userHighIndex = -1, AIHighIndex = -1;
    char userHighSuit = 'x', AIHighSuit = 'x';
    for (int h = 0; h < 7; h++) {
        userHigh = userCards[h].value;
        userHighSuit = userCards[h].suit;
        userHighIndex = h;
        if (userHigh >= 5) { // the lowest straight has a high card of 5
            for (int i = 0; i < 7; i++) {
                if ((userCards[i].value == userHigh - 1) && (userCards[i].suit == userHighSuit)) { // if second high card in straight flush exists
                    for (int j = 0; j < 7; j++) {
                        if ((userCards[j].value == userHigh - 2) && (userCards[j].suit == userHighSuit)) { // if third high card in straight flush exists
                            for (int k = 0; k < 7; k++) {
                                if ((userCards[k].value == userHigh - 3) && (userCards[k].suit == userHighSuit)) { // if fourth high card in straight flush exists
                                    for (int l = 0; l < 7; l++) {
                                        if ((userCards[l].value == userHigh - 4) && (userCards[l].suit == userHighSuit)) { // if fifth high card in straight flush exists
                                            if (userStraight == 0) { // only store if userStraight isn't already found
                                                userStraight = userHigh; // otherwise, 6 card straight flush would lower
                                            }
                                        } else if ((userHigh == 5) && (userCards[l].value == 14) && (userCards[l].suit == userHighSuit)) { // if low card is ace
                                            if (userStraight == 0) {
                                                userStraight = userHigh;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    for (int h = 0; h < 7; h++) {
        AIHigh = AICards[h].value;
        AIHighSuit = AICards[h].suit;
        AIHighIndex = h;
        if (AIHigh >= 5) { // the lowest straight has a high card of 5
            for (int i = 0; i < 7; i++) {
                if ((AICards[i].value == AIHigh - 1) && (AICards[i].suit == AIHighSuit)) { // if second high card in straight flush exists
                    for (int j = 0; j < 7; j++) {
                        if ((AICards[j].value == AIHigh - 2) && (AICards[j].suit == AIHighSuit)) { // if third high card in straight flush exists
                            for (int k = 0; k < 7; k++) {
                                if ((AICards[k].value == AIHigh - 3) && (AICards[k].suit == AIHighSuit)) { // if fourth high card in straight flush exists
                                    for (int l = 0; l < 7; l++) {
                                        if ((AICards[l].value == AIHigh - 4) && (AICards[l].suit == AIHighSuit)) { // if fifth high card in straight flush exists
                                            AIStraight = AIHigh;
                                            AIHigh = 0; // need to break the loop so we don't lower the straight if 6 card straight exists
                                        } else if ((AIHigh == 5) && (AICards[l].value == 14) && (AICards[l].suit == AIHighSuit)) { // if low card is ace
                                            AIStraight = AIHigh;
                                            AIHigh = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (userStraight > AIStraight) {
        cout << "You win!" << endl;
        return 1;
    } else if (userStraight < AIStraight) {
        cout << "Daniel wins!" << endl;
        return 2;
    } else {
        cout << "Tie!" << endl;
        return 0;
    }
    return 0;
}


// Function for resolving a tie between two four of a kinds
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTieFourOfAKind(Card* userCards, Card* AICards) {
    int userFour = 0, AIFour = 0, userHigh = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15, userHighIndex = -1, AIHighIndex = -1;
    while (userHigh != -1) { // FIND USER'S FOUR OF A KIND
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh)) {
                userHigh = userCards[i].value; // find highest value in cards less than previous highest
                userHighIndex = i;
            }
        }
        int numMatches = 0;
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value == userHigh) && (i != userHighIndex)) {
                numMatches += 1; // count how many matches this high card has
            }
        }
        if (numMatches == 3) {
            userFour = userHigh; // if 3 matches, remember this is the four of a kind value
        }
        prevUserHigh = userHigh;
        userHighIndex = -1;
        if (userHigh != 0) {
            userHigh = 0;
        } else {
            userHigh = -1; // break the loop once all cards have been checked
        }
    }
    while (AIHigh != -1) { // FIND AI'S FOUR OF A KIND
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh)) {
                AIHigh = AICards[i].value; // find highest value in cards less than previous highest
                AIHighIndex = i;
            }
        }
        int numMatches = 0;
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value == AIHigh) && (i != AIHighIndex)) {
                numMatches += 1; // count how many matches this high card has
            }
        }
        if (numMatches == 3) {
            AIFour = AIHigh;
        }
        prevAIHigh = AIHigh;
        AIHighIndex = -1;
        if (AIHigh != 0) {
            AIHigh = 0;
        } else {
            AIHigh = -1; // break the loop once all cards have been checked
        }
    }
    if (userFour > AIFour) { // if user's four of a kind is higher than AI's, user wins
        cout << "You win!" << endl;
        return 1;
    } else if (userFour < AIFour) { // if user's four of a kind if lower than AI's, AI wins
        cout << "Daniel wins!" << endl;
        return 2;
    } else { // if four of a kinds are the same, then check fifth card
        userHigh = 0;
        AIHigh = 0;
        for (int i = 0; i < 7; i++) { // find user's and AI's highest card that's not the four of a kind
            if ((userCards[i].value > userHigh) && (userCards[i].value != userFour)) {
                userHigh = userCards[i].value;
            }
            if ((AICards[i].value > AIHigh) && (AICards[i].value != AIFour)) {
                AIHigh = AICards[i].value;
            }
        }
        if (userHigh > AIHigh) { // if user's fifth card is higher than AI's, user wins
            cout << "You win!" << endl;
            return 1;
        } else if (userHigh < AIHigh) { // if user's fifth card if lower than AI's, AI wins
            cout << "Daniel wins!" << endl;
            return 2;
        } else { // if they're the same, tie
            cout << "Tie!" << endl;
            return 0;
        }
    }
    return 0;
}

// Function for resolving a tie between two full houses
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTieFullHouse(Card* userCards, Card* AICards) {
    int userThree = 0, userTwo = 0, userHigh = 0, AIThree = 0, AITwo = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15, userHighIndex = -1, AIHighIndex = -1;
    while (userHigh != -1) { // FIND USER'S FULL HOUSE
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh)) {
                userHigh = userCards[i].value; // find highest value in cards less than previous highest
                userHighIndex = i;
            }
        }
        int numMatches = 0;
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value == userHigh) && (i != userHighIndex)) {
                numMatches += 1; // count how many matches this high card has
            }
        }
        if (numMatches == 2) { // if two matches
            if (userThree == 0) {
                userThree = userHigh; // this is set of three
            } else {
                userTwo = userHigh; // if three already found (and we just found another set of three), this lower set will be the pair
            }
        } else if (numMatches == 1 && (userTwo == 0)) { // if one match (and two not already found), this is the set of two
            userTwo = userHigh;
        }
        prevUserHigh = userHigh;
        userHighIndex = -1;
        if (userHigh != 0) {
            userHigh = 0;
        } else {
            userHigh = -1; // break the loop once all cards have been checked
        }
    }
    while (AIHigh != -1) { // FIND AI'S FULL HOUSE
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh)) {
                AIHigh = AICards[i].value; // find highest value in cards less than previous highest
                AIHighIndex = i;
            }
        }
        int numMatches = 0;
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value == AIHigh) && (i != AIHighIndex)) {
                numMatches += 1; // count how many matches this high card has
            }
        }
        if (numMatches == 2) { // if two matches
            if (AIThree == 0) {
                AIThree = AIHigh; // this is set of three
            } else {
                AITwo = AIHigh; // if three already found (and we just found another set of three), this lower set will be the pair
            }
        } else if (numMatches == 1 && (AITwo == 0)) { // if one match (and two not already found), this is the set of two
            AITwo = AIHigh;
        }
        prevAIHigh = AIHigh;
        AIHighIndex = -1;
        if (AIHigh != 0) {
            AIHigh = 0;
        } else {
            AIHigh = -1; // break the loop once all cards have been checked
        }
    }
    if (userThree > AIThree) { // if user's set of three is higher than AI's, user wins
        cout << "You win!" << endl;
        return 1;
    } else if (userThree < AIThree) { // if user's set of three is lower than AI's, AI wins
        cout << "Daniel wins!" << endl;
        return 2;
    } else { // if the sets of three are the same, check the sets of two
        if (userTwo > AITwo) { // if user's set of two is higher than AI's, user wins
            cout << "You win!" << endl;
            return 1;
        } else if (userTwo < AITwo) { // if user's set of two is lower than AI's, AI wins
            cout << "Daniel wins!" << endl;
            return 2;
        } else { // if both sets of three and sets of two are the same (same full house), true tie
            cout << "Tie!" << endl;
            return 0;
        }
    }
    return 0;
}

// Function for resolving a tie between two flushes
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTieFlush(Card* userCards, Card* AICards) {
    int numHearts = 0, numDiamonds = 0, numSpades = 0, numClubs = 0;
    char flushSuit = 'x';
    for (int i = 0; i < 7; i++) { // count number of each suit and find flush suit
        if (userCards[i].suit == 'H') {
            numHearts += 1;
            if (numHearts >= 5) {
                flushSuit = 'H';
            }
        } else if (userCards[i].suit == 'D') {
            numDiamonds += 1;
            if (numDiamonds >= 5) {
                flushSuit = 'D';
            }
        } else if (userCards[i].suit == 'S') {
            numSpades += 1;
            if (numSpades >= 5) {
                flushSuit = 'S';
            }
        } else {
            numClubs += 1;
            if (numClubs >= 5) {
                flushSuit = 'C';
            }
        }
    }
    int numCard = 1, userHigh = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15;
    while (numCard < 6) { // now compare user's and AI's highest 5 cards that match the flush suit
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh) && (userCards[i].suit == flushSuit)) {
                userHigh = userCards[i].value; // find highest user card
            }
            if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh) && (AICards[i].suit == flushSuit)) {
                AIHigh = AICards[i].value; // find highest AI card
            }
        }
        if (userHigh > AIHigh) {
            cout << "You win!" << endl;
            return 1;
        } else if (userHigh < AIHigh) {
            cout << "Daniel wins!" << endl;
            return 2;
        } else {
            numCard += 1;
            prevUserHigh = userHigh;
            prevAIHigh = AIHigh;
            userHigh = 0;
            AIHigh = 0;
        }
    }
    cout << "Tie!" << endl; // if the user's and AI's top 5 cards in flush suit are the same, it's a tie
    return 0;
}

// Function for resolving a tie between two straights
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTieStraight(Card* userCards, Card* AICards) {
    int userStraight = 0, AIStraight = 0, userHigh = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15, userHighIndex = -1, AIHighIndex = -1;
    while (userHigh != -1) { // FIND USER STRAIGHT
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh)) {
                userHigh = userCards[i].value; // find highest value in cards less than previous highest
                userHighIndex = i;
            }
        }
        if (userHigh >= 5) { // the lowest straight has a high card of 5
            for (int i = 0; i < 7; i++) {
                if (userCards[i].value == userHigh - 1) { // if second high card in straight exists
                    for (int j = 0; j < 7; j++) {
                        if (userCards[j].value == userHigh - 2) { // if third high card in straight exists
                            for (int k = 0; k < 7; k++) {
                                if (userCards[k].value == userHigh - 3) { // if fourth high card in straight exists
                                    for (int l = 0; l < 7; l++) {
                                        if (userCards[l].value == userHigh - 4) { // if fifth high card in straight exists
                                            userStraight = userHigh;
                                            userHigh = 0; // need to break the loop so we don't lower the straight if 6 card straight exists
                                        } else if ((userHigh == 5) && (userCards[l].value == 14)) { // if low card is ace
                                            userStraight = userHigh;
                                            userHigh = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        prevUserHigh = userHigh;
        userHighIndex = -1; // otherwise, reset variables and try again
        if (userHigh != 0) {
            userHigh = 0;
        } else { // break while loop if no new high card found (will never happen as straight is guaranteed)
            userHigh = -1;
        }
    }
    while (AIHigh != -1) { // FIND AI STRAIGHT
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh)) {
                AIHigh = AICards[i].value; // find highest value in cards less than previous highest
                AIHighIndex = i;
            }
        }
        if (AIHigh >= 5) { // the lowest straight has a high card of 5
            for (int i = 0; i < 7; i++) {
                if (AICards[i].value == AIHigh - 1) { // if second high card in straight exists
                    for (int j = 0; j < 7; j++) {
                        if (AICards[j].value == AIHigh - 2) { // if third high card in straight exists
                            for (int k = 0; k < 7; k++) {
                                if (AICards[k].value == AIHigh - 3) { // if fourth high card in straight exists
                                    for (int l = 0; l < 7; l++) {
                                        if (AICards[l].value == AIHigh - 4) { // if fifth high card in straight exists
                                            AIStraight = AIHigh;
                                            AIHigh = 0; // need to break the loop so we don't lower the straight if 6 card straight exists
                                        } else if ((userHigh == 5) && (AICards[l].value == 14)) { // if low card is ace
                                            AIStraight = AIHigh;
                                            AIHigh = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        prevAIHigh = AIHigh;
        AIHighIndex = -1; // otherwise, reset variables and try again
        if (AIHigh != 0) {
            AIHigh = 0;
        } else { // break while loop if no new high card found (will never happen as straight is guaranteed)
            AIHigh = -1;
        }
    }
    if (userStraight > AIStraight) {
        cout << "You win!" << endl;
        return 1;
    } else if (userStraight < AIStraight) {
        cout << "Daniel wins!" << endl;
        return 2;
    } else {
        cout << "Tie!" << endl;
        return 0;
    }
    return 0;
}


// Function for resolving a tie between two three of a kinds
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTieThreeOfAKind(Card* userCards, Card* AICards) {
    int userSet = 0, userHigh = 0, AISet = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15, userHighIndex = -1, AIHighIndex = -1;
    while (userHigh != -1) { // FIND USER SET
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh)) {
                userHigh = userCards[i].value; // find highest value in cards less than previous highest
                userHighIndex = i;
            }
        }
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value == userHigh) && (i != userHighIndex)) {
                userSet = userHigh; // remember the set, if found (only need to check for one match since we know hand has 3oaK, not FH
            }
        }
        prevUserHigh = userHigh;
        userHighIndex = -1;
        if (userHigh != 0) {
            userHigh = 0;
        } else {
            userHigh = -1; // break the loop if no high card was found (although this won't ever happen because pair is guaranteed)
        }
    }
    while (AIHigh != -1) { // FIND AI SET
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh)) {
                AIHigh = AICards[i].value; // find highest value in cards less than previous highest
                AIHighIndex = i;
            }
        }
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value == AIHigh) && (i != AIHighIndex)) {
                AISet = AIHigh; // // remember the set, if found (only need to check for one match since we know hand has 3oaK, not FH
            }
        }
        prevAIHigh = AIHigh;
        AIHighIndex = -1;
        if (AIHigh != 0) {
            AIHigh = 0;
        } else {
            AIHigh = -1; // break the loop if no high card was found (although this won't ever happen because set is guaranteed)
        }
    }
    if (userSet > AISet) { // if user's set is higher than AI's set
        cout << "You win!" << endl;
        return 1;
    } else if (userSet < AISet) { // if AI's set is higher than user's set
        cout << "Daniel wins!" << endl;
        return 2;
    } else { // if the set is the same, we have to check the two kickers (only occurs with 2/3 on board, other 2 in hands)
        prevUserHigh = 15;
        prevAIHigh = 15;
        userHigh = 0;
        AIHigh = 0; // reset searching variables
        userHighIndex = -1;
        AIHighIndex = -1;
        int numCard = 1;
        while (numCard < 3) {
            for (int i = 0; i < 7; i++) {
                if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh) && (userCards[i].value != userSet)) {
                    userHigh = userCards[i].value; // find highest user card, excluding the paired cards
                }
                if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh) && (AICards[i].value != AISet)) {
                    AIHigh = AICards[i].value; // find highest AI card, excluding the paired cards
                }
            }
            if (userHigh > AIHigh) {
                cout << "You win!" << endl;
                return 1;
            } else if (userHigh < AIHigh) {
                cout << "Daniel wins!" << endl;
                return 2;
            } else {
                numCard += 1;
                prevUserHigh = userHigh;
                prevAIHigh = AIHigh;
                userHigh = 0;
                AIHigh = 0;
            }
        }
    }
    cout << "Tie!" << endl; // if each of the user's and AI's set and next highest 2 cards are the same, the hand is a tie
    return 0;
}


// Function for resolving a tie between two two pairs
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTieTwoPair(Card* userCards, Card* AICards) {
    int userFirstPair = 0, userSecondPair = 0, userHigh = 0, AIFirstPair = 0, AISecondPair = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15, userHighIndex = -1, AIHighIndex = -1;
    while (userHigh != -1) { // FIND USER'S HIGH PAIR
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh)) {
                userHigh = userCards[i].value; // find highest value in cards less than previous highest
                userHighIndex = i;
            }
        }
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value == userHigh) && (i != userHighIndex) && (userFirstPair == 0)) {
                userFirstPair = userHigh; // remember the pair, if found
            }
        }
        prevUserHigh = userHigh;
        userHighIndex = -1;
        if (userHigh != 0) {
            userHigh = 0;
        } else {
            userHigh = -1; // break the loop if no high card was found (although this won't ever happen because pair is guaranteed)
        }
    }
    while (AIHigh != -1) { // FIND AI'S HIGH PAIR
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh)) {
                AIHigh = AICards[i].value; // find highest value in cards less than previous highest
                AIHighIndex = i;
            }
        }
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value == AIHigh) && (i != AIHighIndex) && (AIFirstPair == 0)) {
                AIFirstPair = AIHigh; // remember the pair, if found
            }
        }
        prevAIHigh = AIHigh;
        AIHighIndex = -1;
        if (AIHigh != 0) {
            AIHigh = 0;
        } else {
            AIHigh = -1; // break the loop if no high card was found (although this won't ever happen because pair is guaranteed)
        }
    }
    if (userFirstPair > AIFirstPair) { // if user's high pair is higher than AI's pair
        cout << "You win!" << endl;
        return 1;
    } else if (userFirstPair < AIFirstPair) { // if AI's high pair is higher than user's pair
        cout << "Daniel wins!" << endl;
        return 2;
    } else { // if high pairs are the same, we have to check the low pair
        prevUserHigh = 15;
        prevAIHigh = 15;
        userHigh = 0;
        AIHigh = 0; // reset searching variables
        userHighIndex = -1;
        AIHighIndex = -1;
        while (userHigh != -1) { // FIND USER'S LOW HIGH PAIR
            for (int i = 0; i < 7; i++) {
                if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh) && (userCards[i].value != userFirstPair)) {
                    userHigh = userCards[i].value; // find highest value in cards less than previous highest
                    userHighIndex = i;
                }
            }
            for (int i = 0; i < 7; i++) {
                if ((userCards[i].value == userHigh) && (i != userHighIndex) && (userSecondPair == 0)) {
                    userSecondPair = userHigh; // remember the pair, if found
                }
            }
            prevUserHigh = userHigh;
            userHighIndex = -1;
            if (userHigh != 0) {
                userHigh = 0;
            } else {
                userHigh = -1; // break the loop if no high card was found (although this won't ever happen because pair is guaranteed)
            }
        }
        while (AIHigh != -1) { // FIND AI'S LOW HIGH PAIR
            for (int i = 0; i < 7; i++) {
                if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh) && (AICards[i].value != AIFirstPair)) {
                    AIHigh = AICards[i].value; // find highest value in cards less than previous highest
                    AIHighIndex = i;
                }
            }
            for (int i = 0; i < 7; i++) {
                if ((AICards[i].value == AIHigh) && (i != AIHighIndex) && (AISecondPair == 0)) {
                    AISecondPair = AIHigh; // remember the pair, if found
                }
            }
            prevAIHigh = AIHigh;
            AIHighIndex = -1;
            if (AIHigh != 0) {
                AIHigh = 0;
            } else {
                AIHigh = -1; // break the loop if no high card was found (although this won't ever happen because pair is guaranteed)
            }
        }
        if (userSecondPair > AISecondPair) { // if user's high pair is higher than AI's pair
            cout << "You win!" << endl;
            return 1;
        } else if (userSecondPair < AISecondPair) { // if AI's high pair is higher than user's pair
            cout << "Daniel wins!" << endl;
            return 2;
        } else { // if BOTH pairs have now proven to be the same, we check the 5th card kicker
            userHigh = 0;
            AIHigh = 0; // reset searching variables
            userHighIndex = -1;
            AIHighIndex = -1;
            for (int i = 0; i < 7; i++) {
                if ((userCards[i].value > userHigh) && (userCards[i].value != userFirstPair) && (userCards[i].value != userSecondPair)) {
                    userHigh = userCards[i].value; // find highest user card, excluding the two pairs
                }
                if ((AICards[i].value > AIHigh) && (AICards[i].value != AIFirstPair) && (AICards[i].value != AISecondPair)) {
                    AIHigh = AICards[i].value; // find highest AI card, excluding the two pairs
                }
            }
            if (userHigh > AIHigh) {
                cout << "You win!" << endl;
                return 1;
            } else if (userHigh < AIHigh) {
                cout << "Daniel wins!" << endl;
                return 2;
            } else {
                cout << "Tie!" << endl;
                return 0; // if user's and AI's two pairs and 5th card are all the same, it's a tie
            }
        }
    }
    return 0; // I don't think the function will ever get here
}



// Function for resolving a tie between two pairs
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTiePair(Card* userCards, Card* AICards) {
    int userPair = 0, userHigh = 0, AIPair = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15, userHighIndex = -1, AIHighIndex = -1;
    while (userHigh != -1) { // FIND USER PAIR
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh)) {
                userHigh = userCards[i].value; // find highest value in cards less than previous highest
                userHighIndex = i;
            }
        }
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value == userHigh) && (i != userHighIndex)) {
                userPair = userHigh; // remember the pair, if found
            }
        }
        prevUserHigh = userHigh;
        userHighIndex = -1;
        if (userHigh != 0) {
            userHigh = 0;
        } else {
            userHigh = -1; // break the loop if no high card was found (although this won't ever happen because pair is guaranteed)
        }
    }
    while (AIHigh != -1) { // FIND AI PAIR
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh)) {
                AIHigh = AICards[i].value; // find highest value in cards less than previous highest
                AIHighIndex = i;
            }
        }
        for (int i = 0; i < 7; i++) {
            if ((AICards[i].value == AIHigh) && (i != AIHighIndex)) {
                AIPair = AIHigh; // remember the pair, if found
            }
        }
        prevAIHigh = AIHigh;
        AIHighIndex = -1;
        if (AIHigh != 0) {
            AIHigh = 0;
        } else {
            AIHigh = -1; // break the loop if no high card was found (although this won't ever happen because pair is guaranteed)
        }
    }
    if (userPair > AIPair) { // if user's pair is higher than AI's pair
        cout << "You win!" << endl;
        return 1;
    } else if (userPair < AIPair) { // if AI's pair is higher than user's pair
        cout << "Daniel wins!" << endl;
        return 2;
    } else { // if pairs are the same, we have to check the three kickers
        prevUserHigh = 15;
        prevAIHigh = 15;
        userHigh = 0;
        AIHigh = 0; // reset searching variables
        userHighIndex = -1;
        AIHighIndex = -1;
        int numCard = 1;
        while (numCard < 4) {
            for (int i = 0; i < 7; i++) {
                if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh) && (userCards[i].value != userPair)) {
                    userHigh = userCards[i].value; // find highest user card, excluding the paired cards
                }
                if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh) && (AICards[i].value != AIPair)) {
                    AIHigh = AICards[i].value; // find highest AI card, excluding the paired cards
                }
            }
            if (userHigh > AIHigh) {
                cout << "You win!" << endl;
                return 1;
            } else if (userHigh < AIHigh) {
                cout << "Daniel wins!" << endl;
                return 2;
            } else {
                numCard += 1;
                prevUserHigh = userHigh;
                prevAIHigh = AIHigh;
                userHigh = 0;
                AIHigh = 0;
            }
        }
    }
    cout << "Tie!" << endl; // if each of the user's and AI's pairs and next highest 3 cards are the same, the hand is a tie
    return 0;
}

// Function for resolving a tie between two high cards
// Returns 1 if user wins, 2 if AI wins, and 0 if it is a true tie
int GameManager::resolveTieHighCard(Card* userCards, Card* AICards) {
    int numCard = 1, userHigh = 0, AIHigh = 0, prevUserHigh = 15, prevAIHigh = 15;
    while (numCard < 6) {
        for (int i = 0; i < 7; i++) {
            if ((userCards[i].value > userHigh) && (userCards[i].value < prevUserHigh)) {
                userHigh = userCards[i].value; // find highest user card
            }
            if ((AICards[i].value > AIHigh) && (AICards[i].value < prevAIHigh)) {
                AIHigh = AICards[i].value; // find highest AI card
            }
        }
        if (userHigh > AIHigh) {
            cout << "You win!" << endl;
            return 1;
        } else if (userHigh < AIHigh) {
            cout << "Daniel wins!" << endl;
            return 2;
        } else {
            numCard += 1;
            prevUserHigh = userHigh;
            prevAIHigh = AIHigh;
            userHigh = 0;
            AIHigh = 0;
        }
    }
    cout << "Tie!" << endl; // if each of the user's and AI's top 5 cards are the same, the hand is a tie
    return 0;
}


// Function for initializing the deck
// Creates a deck of 53 Card objects, consisting of the 52 cards in a real card deck,
// and then a 53rd "dead" card that is used for some of the GameManager logic
void GameManager::initDeck() {
    potSize = 0;
    userStack = 200;
    AIStack = 200;
    (deck+0)->value = 14;
    (deck+0)->suit = 'H';
    (deck+1)->value = 14;
    (deck+1)->suit = 'D';
    (deck+2)->value = 14;
    (deck+2)->suit = 'S';
    (deck+3)->value = 14;
    (deck+3)->suit = 'C';
    (deck+4)->value = 2;
    (deck+4)->suit = 'H';
    (deck+5)->value = 2;
    (deck+5)->suit = 'D';
    (deck+6)->value = 2;
    (deck+6)->suit = 'S';
    (deck+7)->value = 2;
    (deck+7)->suit = 'C';
    (deck+8)->value = 3;
    (deck+8)->suit = 'H';
    (deck+9)->value = 3;
    (deck+9)->suit = 'D';
    (deck+10)->value = 3;
    (deck+10)->suit = 'S';
    (deck+11)->value = 3;
    (deck+11)->suit = 'C';
    (deck+12)->value = 4;
    (deck+12)->suit = 'H';
    (deck+13)->value = 4;
    (deck+13)->suit = 'D';
    (deck+14)->value = 4;
    (deck+14)->suit = 'S';
    (deck+15)->value = 4;
    (deck+15)->suit = 'C';
    (deck+16)->value = 5;
    (deck+16)->suit = 'H';
    (deck+17)->value = 5;
    (deck+17)->suit = 'D';
    (deck+18)->value = 5;
    (deck+18)->suit = 'S';
    (deck+19)->value = 5;
    (deck+19)->suit = 'C';
    (deck+20)->value = 6;
    (deck+20)->suit = 'H';
    (deck+21)->value = 6;
    (deck+21)->suit = 'D';
    (deck+22)->value = 6;
    (deck+22)->suit = 'S';
    (deck+23)->value = 6;
    (deck+23)->suit = 'C';
    (deck+24)->value = 7;
    (deck+24)->suit = 'H';
    (deck+25)->value = 7;
    (deck+25)->suit = 'D';
    (deck+26)->value = 7;
    (deck+26)->suit = 'S';
    (deck+27)->value = 7;
    (deck+27)->suit = 'C';
    (deck+28)->value = 8;
    (deck+28)->suit = 'H';
    (deck+29)->value = 8;
    (deck+29)->suit = 'D';
    (deck+30)->value = 8;
    (deck+30)->suit = 'S';
    (deck+31)->value = 8;
    (deck+31)->suit = 'C';
    (deck+32)->value = 9;
    (deck+32)->suit = 'H';
    (deck+33)->value = 9;
    (deck+33)->suit = 'D';
    (deck+34)->value = 9;
    (deck+34)->suit = 'S';
    (deck+35)->value = 9;
    (deck+35)->suit = 'C';
    (deck+36)->value = 10;
    (deck+36)->suit = 'H';
    (deck+37)->value = 10;
    (deck+37)->suit = 'D';
    (deck+38)->value = 10;
    (deck+38)->suit = 'S';
    (deck+39)->value = 10;
    (deck+39)->suit = 'C';
    (deck+40)->value = 11;
    (deck+40)->suit = 'H';
    (deck+41)->value = 11;
    (deck+41)->suit = 'D';
    (deck+42)->value = 11;
    (deck+42)->suit = 'S';
    (deck+43)->value = 11;
    (deck+43)->suit = 'C';
    (deck+44)->value = 12;
    (deck+44)->suit = 'H';
    (deck+45)->value = 12;
    (deck+45)->suit = 'D';
    (deck+46)->value = 12;
    (deck+46)->suit = 'S';
    (deck+47)->value = 12;
    (deck+47)->suit = 'C';
    (deck+48)->value = 13;
    (deck+48)->suit = 'H';
    (deck+49)->value = 13;
    (deck+49)->suit = 'D';
    (deck+50)->value = 13;
    (deck+50)->suit = 'S';
    (deck+51)->value = 13;
    (deck+51)->suit = 'C';
    (deck+52)->value = -1;
    (deck+52)->suit = 'X';
}
