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


class AI {
public:
    int userRange[1326][2]; // there are 1326 possible 2 card hands
    float handStrengths[1326];
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
// Returns -1 if the AI decides to fold, 0 if the AI decides to check, and otherwise an integer that represents
// the amount that the AI decides to bet (whether that is a call or a raise is shown with cout, and is handled in GameManager)
int AI::makeBetDecision(int currBet, int AILastBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck) {
    cout << "Daniel is thinking..." << endl << endl;
    usleep(3000000);
    float confidenceRatio = determineHandStrength(currBet, potSize, AIStack, userStack, AIHand, boardCards, betRound, deck);
    if ((currBet-AILastBet) == 0) { // if AI is first bet or user has checked
        if (confidenceRatio > 0.50) {
            int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
            cout << "Daniel bets $" << bet << "." << endl;
            return bet;
        } else {
            cout << "Daniel checks." << endl;
            return 0;
        }
    } else { // if user bets into AI
        if (confidenceRatio > 0.75) {
            int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
            cout << "Daniel raises to $" << currBet+bet << "." << endl;
            return currBet+bet-AILastBet;
        } else if (confidenceRatio > 0.4) {
            cout << "Daniel calls, putting in $" << currBet-AILastBet << "." << endl;
            return currBet-AILastBet;
        } else {
            cout << "Daniel folds." << endl;
            return -1;
        }
    }
    return 0.0;
}


// AI function for determining hand strength, and a confidence level that the AI has a better hand than the user


// AI function for deciding if the AI wants to raise
// Returns -1 if the AI does not wish to raise, or returns an integer indicating the amount the AI wants to raise
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
        return (float)(rand()%101)/100;
    } else if (betRound == 2) { // turn
        return (float)(rand()%101)/100;
    } else { // river
        return (float)(rand()%101)/100;
    }
    return 0.0;
}


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
        return (float)(rand()%101)/100;
    } else if (betRound == 2) { // turn
        return (float)(rand()%101)/100;
    } else { // river
        return (float)(rand()%101)/100;
    }
    return 0.0;
}





int AI::determineBetSize(int currBet, int AILastBet, int potSize, int AIStack, int userStack, float confidenceRatio) {
    int bet = (int)(confidenceRatio * potSize);
    if (bet < 2) {
        bet = 2;
    } else if (bet < 2*currBet) {
        bet = 2*currBet;
    }
    return bet;
}




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
