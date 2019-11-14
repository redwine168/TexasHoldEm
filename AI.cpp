//
//  AI.cpp
//  Texas Hold 'em
//
//  Created by Jonathan Redwine on 10/10/19.
//  Copyright © 2019 JonathanRedwine. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include "Card.cpp"
using namespace std;


// AI Class for playing Texas Hold 'em
// Objects of the AI class have the following attributes:
//  userRange: a 2D array of ints, referring to indices of the deck of Card objects in the GameManager class.
//      This is how the AI tracks the hands that the user could possibly have throughout a hand
//  handStrengths: an array of floats of strengths of hands corresponding to the hands in userRange
// The idea of the AI is that it bases its decisions on how strong its own hand is compared to the hands
//  that the AI thinks the user could have.  For example, at the beginning of the hand, the AI compares its hand
//  to all 1326 2 card combinations that the user could have.  If the user bets, the AI might consider that the user
//  wouldn't have the lower third (by hand strength) of the 2 card combinations.  Thus, in future considerations, the
//  AI would only compare its own hand to about ~900 2 card combinations.  Based on how confident the AI is in its own hand
//  compared to the user's possible hands, as well as the size of the current bet and the pot size, the AI will make
//  decisions on whether to check, bet, call, raise, and fold.
class AI {
public:
    int userRange[1326][2]; // there are 1326 possible 2 card hands
    float handStrengths[1326];
    // Default (and only) constructor for AI objects
    AI() {
        resetUserRange();
    }
    int makeBetDecision(int currBet, int AILastBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck);
    float determineHandStrength(int currBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck);
    float determineFlushOdds(Card* cards, int betRound);
    float determineStraightOdds(Card* cards, int betRound);
    float determineGoodPairOdds(Card* cards, int betRound);
    int determineBetSize(int currBet, int AILastBet, int potSize, int AIStack, int userStack, float confidenceRatio);
    void resetUserRange();
};


// AI method for making a bet decision.  The parameters are essentially everything relevant on the poker table.
//  the current bet, the last bet the AI made, the pot size, the AI's stack, the user's stack, the AI's cards,
//  the cards on the board, and what round of betting we're on
// Returns -1 if the AI decides to fold, 0 if the AI decides to check, and otherwise an integer that represents
// the amount that the AI decides to bet (whether that is a call or a raise is shown with cout, and is handled in GameManager)
int AI::makeBetDecision(int currBet, int AILastBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck) {
    cout << "Daniel is thinking..." << endl << endl;
    usleep(3000000);
    float confidenceRatio = determineHandStrength(currBet, potSize, AIStack, userStack, AIHand, boardCards, betRound, deck);
    if ((currBet-AILastBet) == 0) { // if AI is first bet or user has checked
        if (confidenceRatio > 0.50) { // arbitrary threshold for if the AI wishes to bet
            int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
            cout << "Daniel bets $" << bet << "." << endl;
            return bet;
        } else {
            cout << "Daniel checks." << endl;
            return 0;
        }
    } else { // if user bets into AI
        if (confidenceRatio > 0.75) { // arbitrary threshold for if the AI wishes to raise
            int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
            cout << "Daniel raises to $" << currBet+bet << "." << endl;
            return currBet+bet-AILastBet;
        } else if (confidenceRatio > 0.4) { // arbitrary threshold for if the AI wishes to call
            cout << "Daniel calls, putting in $" << currBet-AILastBet << "." << endl;
            return currBet-AILastBet;
        } else {
            cout << "Daniel folds." << endl;
            return -1;
        }
    }
    return 0.0;
}


// AI function for determining how strong it thinks its hand is.  It calculates the strength of its own hand, and then the strengths
// of all the hands it thinks the user could have.  It determines the percent of user hands that its own hand beats, and returns that
// ratio
float AI::determineHandStrength(int currBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck) {
    Card* playableCards = new Card[7];
    for (int i = 0; i < 2; i++) {
        playableCards[i] = AIHand[i];
    }
    for (int i = 2; i < 7; i++) {
        playableCards[i] = boardCards[i-2];
    }
    float AIHandStrength = determineFlushOdds(playableCards, betRound) + determineStraightOdds(playableCards, betRound) + determineGoodPairOdds(playableCards, betRound);
    int numWorseHands = 0;
    int numTotalHands = 0;
    float confidenceRatio = 0.0;
    float userHandStrength = 0.0;
    for (int i = 0; i < 1326; i += 1) { // go through all possible two card hands
        if (deck[userRange[i][0]].value != -1) { // all of the hands that AI has decided user could still have
            playableCards[0] = deck[userRange[i][0]];
            playableCards[1] = deck[userRange[i][1]];
            userHandStrength = determineFlushOdds(playableCards, betRound) + determineStraightOdds(playableCards, betRound) + determineGoodPairOdds(playableCards, betRound); // determine how strong this hand is
            handStrengths[i] = userHandStrength; // store this hand strength
            if (userHandStrength <= AIHandStrength) { // if AI has a better hand, count it (num hands AI beats)
                numWorseHands += 1;
            }
            numTotalHands += 1; // count total hands user could have
        }
    }
    confidenceRatio = (float)(numWorseHands)/(float)(numTotalHands);
    return confidenceRatio;
}


// Function for determining the AI's bet size (when the AI wants to raise).  This decision is based on how confident the AI
// is in its hand, the current bet, the pot size, etc.
// Returns an int referring to the amount the AI wishes to bet
int AI::determineBetSize(int currBet, int AILastBet, int potSize, int AIStack, int userStack, float confidenceRatio) {
    int bet = (int)(confidenceRatio * potSize);
    if (bet < 2) {
        bet = 2;
    } else if (bet < 2*currBet) {
        bet = 2*currBet;
    }
    return bet;
}



// Determines how likely it is for the input cards to achieve a flush
// The parameter "cards" contains the two hole cards (whether that be the AI's cards or the two cards of a user's possible hand),
// and the current board cards.  Based on the parameter betRound (referring to if it's pre-flop, on the turn, etc), the flush
// likelihood is determined differently.
// Returns an arbitrary float based on how close to a flush these cards are
float AI::determineFlushOdds(Card* cards, int betRound) {
    if (betRound == 0) { // pre-flop
        if (cards[0].suit == cards[1].suit) {
            return 0.80; // if the AI's two cards are suited pre-flop, return 1.0
        } else {
            return 0.0; // if they're not, return 0.0
        }
    } else if (betRound == 1) { // flop
        int numHearts = 0, numDiamonds = 0, numSpades = 0, numClubs = 0;
        for (int i = 0; i < 5; i++) { // count number of each suit there are
            if (cards[i].suit == 'H') {
                numHearts += 1;
            } else if (cards[i].suit == 'D') {
                numDiamonds += 1;
            } else if (cards[i].suit == 'S') {
                numSpades += 1;
            } else {
                numClubs += 1;
            }
        }
        if ((numHearts == 5) || (numDiamonds == 5) || (numSpades == 5) || (numClubs == 5)) { // if already made flush
            return 2.0;
        } else if ((numHearts == 4) || (numDiamonds == 4) || (numSpades == 4) || (numClubs == 4)) { // if 4/5 suited cards
            return 1.25;
        } else if ((numHearts == 3) || (numDiamonds == 3) || (numSpades == 3) || (numClubs == 3)) { // if 3/5 suited cards
            return 0.25;
        } else { // if only 2/5 suited cards (this is the minimum at the flop, flush is impossible)
            return 0.0;
        }
    } else if (betRound == 2) { // turn
        int numHearts = 0, numDiamonds = 0, numSpades = 0, numClubs = 0;
        for (int i = 0; i < 6; i++) { // count number of each suit there are
            if (cards[i].suit == 'H') {
                numHearts += 1;
            } else if (cards[i].suit == 'D') {
                numDiamonds += 1;
            } else if (cards[i].suit == 'S') {
                numSpades += 1;
            } else {
                numClubs += 1;
            }
        }
        if ((numHearts >= 5) || (numDiamonds >= 5) || (numSpades >= 5) || (numClubs >= 5)) { // if already made flush
            return 2.0;
        } else if ((numHearts == 4) || (numDiamonds == 4) || (numSpades == 4) || (numClubs == 4)) { // if 4/5 suited cards
            return 1.25;
        } else { // if less than 4/5 suited cards (if so at the turn, flush is impossible)
            return 0.0;
        }
    } else { // river
        int numHearts = 0, numDiamonds = 0, numSpades = 0, numClubs = 0;
        for (int i = 0; i < 7; i++) { // count number of each suit there are
            if (cards[i].suit == 'H') {
                numHearts += 1;
            } else if (cards[i].suit == 'D') {
                numDiamonds += 1;
            } else if (cards[i].suit == 'S') {
                numSpades += 1;
            } else {
                numClubs += 1;
            }
        }
        if ((numHearts >= 5) || (numDiamonds >= 5) || (numSpades >= 5) || (numClubs >= 5)) { // if made flush
            return 2.0;
        } else { // no flush
            return 0.0;
        }
    }
    return 0.0;
}


// Determines how likely it is for the input cards to achieve a straight
// The parameter "cards" contains the two hole cards (whether that be the AI's cards or the two cards of a user's possible hand),
// and the current board cards.  Based on the parameter betRound (referring to if it's pre-flop, on the turn, etc), the straight
// likelihood is determined differently.
// Returns an arbitrary float based on how close to a straight these cards are
float AI::determineStraightOdds(Card* cards, int betRound) {
    if (betRound == 0) { // pre-flop
        if (abs(cards[0].value - cards[1].value) == 0) { // if the two cards have same value
            return 0.0;
        } else if (abs(cards[0].value - cards[1].value) == 1) { // if two adjacent cards
            return 1.0;
        } else if (abs(cards[0].value - cards[1].value) < 5) { // if cards separated by 2-4
            return 0.5;
        } else if (abs(cards[0].value - cards[1].value) > 8 ) { // if cards separated by greater than 8 (i.e., maybe an ace in a possible straight hand))
            int firstVal = cards[0].value;
            int secondVal = cards[1].value;
            if (firstVal == 14) { // update first value if it's an ace
                firstVal = 1;
            }
            if (secondVal == 14) { // update second value if it's an ace
                secondVal = 1;
            }
            if ((firstVal - secondVal) == 1) { // ace - two
                return 1.0;
            } else if ((firstVal - secondVal) < 5) { // ace - three, four, or five
                return 0.5;
            } else {
                return 0.0;
            }
        } else { // if cards separated by 5-8 (definitely no straight possible)
            return 0.0;
        }
    } else if (betRound == 1) { // flop
        float bestVal = 0.0;
        for (int i = 0; i < 5; i++) { // test each card as top card of straight
            for (int j = 0; j < 5; j++) { // see if first card below exists
                if (cards[j].value == cards[i].value - 1) {
                    if (bestVal < 0.2) {
                        bestVal = 0.2;
                    }
                    for (int k = 0; k < 5; k++) { // see if second card below exists
                        if (cards[k].value == cards[i].value - 2) {
                            if (bestVal < 0.4) {
                                bestVal = 0.4;
                            }
                            for (int l = 0; l < 5; l++) { // see if third cards below exists
                                if (cards[l].value == cards[i].value - 3) {
                                    if (bestVal < 0.6) {
                                        bestVal = 0.6;
                                    }
                                    for (int m = 0; m < 5; m++) { // see if straight is already made
                                        if (bestVal < 1.0) {
                                            bestVal = 1.0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
        }
        return bestVal;
    } else if (betRound == 2) { // turn
        float bestVal = 0.0;
        for (int i = 0; i < 6; i++) { // test each card as top card of straight
            for (int j = 0; j < 6; j++) { // see if first card below exists
                if (cards[j].value == cards[i].value - 1) {
                    if (bestVal < 0.2) {
                        bestVal = 0.2;
                    }
                    for (int k = 0; k < 6; k++) { // see if second card below exists
                        if (cards[k].value == cards[i].value - 2) {
                            if (bestVal < 0.4) {
                                bestVal = 0.4;
                            }
                            for (int l = 0; l < 6; l++) { // see if third cards below exists
                                if (cards[l].value == cards[i].value - 3) {
                                    if (bestVal < 0.6) {
                                        bestVal = 0.6;
                                    }
                                    for (int m = 0; m < 6; m++) { // see if straight is already made
                                        if (bestVal < 1.0) {
                                            bestVal = 1.0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
        }
        return bestVal;
    } else { // river
        float bestVal = 0.0;
        for (int i = 0; i < 7; i++) { // test each card as top card of straight
            for (int j = 0; j < 7; j++) { // see if first card below exists
                if (cards[j].value == cards[i].value - 1) {
                    if (bestVal < 0.2) {
                        bestVal = 0.2;
                    }
                    for (int k = 0; k < 7; k++) { // see if second card below exists
                        if (cards[k].value == cards[i].value - 2) {
                            if (bestVal < 0.4) {
                                bestVal = 0.4;
                            }
                            for (int l = 0; l < 7; l++) { // see if third cards below exists
                                if (cards[l].value == cards[i].value - 3) {
                                    if (bestVal < 0.6) {
                                        bestVal = 0.6;
                                    }
                                    for (int m = 0; m < 7; m++) { // see if straight is already made
                                        if (bestVal < 1.0) {
                                            bestVal = 1.0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
        }
        return bestVal;
    }
    return 0.0;
}


// Determines how likely it is for the input cards to achieve a good pair (the higher the pair the better)
// The parameter "cards" contains the two hole cards (whether that be the AI's cards or the two cards of a user's possible hand),
// and the current board cards.  Based on the parameter betRound (referring to if it's pre-flop, on the turn, etc), the high pair
// likelihood is determined differently.
// Returns an arbitrary float based on how close to a high pair these cards are
float AI::determineGoodPairOdds(Card* cards, int betRound) {
    if (betRound == 0) { // pre-flop
        if (cards[0].value == cards[1].value) { // if the AI has a pocket pair
            if (cards[0].value == 14) { // pad odds if pocket aces
                return 3.0;
            } else if (cards[0].value == 13) { // and if pocket kings
                return 2.75;
            } else if (cards[0].value == 12) { // and if pocket queens
                return 2.5;
            } else {
                return 1.75;
            }
        } else {
            float cardSum = cards[0].value + cards[1].value;
            cardSum = (float)(cardSum/27*2); // highest sum, of not paired cards, would be 14 + 13 (ace king) so we normalize across that range.  I then multiply 2 as a custom factor to get the AI to play more hands with high cards
            return cardSum;
        }
    } else if (betRound == 1) { // flop
        return (float)(rand()%101)/100; // THESE ARE CURRENTLY RANDOM AS I HAVE NOT PROGRAMMED THE AI IN YET
    } else if (betRound == 2) { // turn
        return (float)(rand()%101)/100; // I MAKE THEM RANDOM SO THE GAME IS PLAYABLE, BUT THESE WILL CHANGE
    } else { // river
        return (float)(rand()%101)/100;
    }
    return 0.0;
}




// Function for resetting the user's range
// This will happen at the beginning of the hand, restoring the user's possible hands to all 1326 possiblities
void AI::resetUserRange() {
    int x = 0;
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            userRange[x][0] = i;
            userRange[x][1] = j;
            handStrengths[x] = 0.0;
            x += 1;
        }
    }
}
