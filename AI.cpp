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
#include <random>
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
    int possibleUserHands;
    // Default (and only) constructor for AI objects
    AI() {
        resetUserRange();
    }
    int makeBetDecision(int currBet, int AILastBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck);
    float removeHandsFromRange(int currBet, int AILastBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck);
    float determineHandStrength(int currBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck);
    float determineFlushOdds(Card* cards, int betRound);
    float determineStraightOdds(Card* cards, int betRound);
    float determineGoodPairOdds(Card* cards, int betRound);
    float determineStraightFlushOdds(Card* cards, int betRound);
    float determineFourOfAKindOdds(Card* cards, int betRound);
    float determineFullHouseOdds(Card* cards, int betRound);
    float determineThreeOfAKindOdds(Card* cards, int betRound);
    float determineTwoPairOdds(Card* cards, int betRound);
    int determineBetSize(int currBet, int AILastBet, int potSize, int AIStack, int userStack, float confidenceRatio);
    void resetUserRange();
};


// AI method for making a bet decision.  The parameters are essentially everything relevant on the poker table.
//  the current bet, the last bet the AI made, the pot size, the AI's stack, the user's stack, the AI's cards,
//  the cards on the board, and what round of betting we're on
// Returns -1 if the AI decides to fold, 0 if the AI decides to check, and otherwise an integer that represents
// the amount that the AI decides to bet (whether that is a call or a raise is shown with cout, and is handled in GameManager)
int AI::makeBetDecision(int currBet, int AILastBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck) {
    // if amount owed is more than AI's stack, make amount owed equal to AI's stack
    if ((currBet - AILastBet) > AIStack) {
        currBet = AILastBet + AIStack;
    }
    // first determine hand strength, so that user's range has its strengths established
    determineHandStrength(currBet, potSize, AIStack, userStack, AIHand, boardCards, betRound, deck);
    // remove hands from range, according to user's bet
    removeHandsFromRange(currBet, AILastBet, potSize, AIStack, userStack, AIHand, boardCards, betRound, deck);
    cout << "Daniel is thinking..." << endl << endl;
    usleep(3000000);
    float confidenceRatio = determineHandStrength(currBet, potSize, AIStack, userStack, AIHand, boardCards, betRound, deck);
    int amountOwed = currBet - AILastBet;
    int payout = potSize + amountOwed;    
    if ((currBet-AILastBet) == 0) { // if AI is first bet or user has checked
        int rando = rand()%100;
        // if AI less than 0.25 confident
        if (confidenceRatio <= 0.25) {
            // always check
            cout << "Daniel checks." << endl;
            return 0;
        }
        // if AI 0.25-0.50 confident
        else if (confidenceRatio <= 0.5) {
            // 1/3 of the time bet
            if (rando <= 33) {
                int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
                // Remove some hands here, because if the user just calls then the AI needs to update
                removeHandsFromRange(currBet+bet, currBet, potSize+bet+AILastBet, AIStack, userStack, AIHand, boardCards, betRound, deck);
                return bet;
            } else { // 2/3 of the time check
                cout << "Daniel checks." << endl;
                return 0;
            }
        }
        // if AI 0.50-0.75 confident
        else if (confidenceRatio <= 0.75) {
            if (rando <= 67) { // 2/3 of the time bet
                int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
                // Remove some hands here, because if the user just calls then the AI needs to update
                removeHandsFromRange(currBet+bet, currBet, potSize+bet, AIStack, userStack, AIHand, boardCards, betRound, deck);
                return bet;
            } else { // 1/3 of the time check
                cout << "Daniel checks." << endl;
                return 0;
            }
        }
        // if AI is greater than 0.75 confident
        else {
            // always bet
            int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
            // Remove some hands here, because if the user just calls then the AI needs to update
            removeHandsFromRange(currBet+bet, currBet, potSize+bet, AIStack, userStack, AIHand, boardCards, betRound, deck);
            return bet;
        }
    }
    
    // if user bets into AI
    else {
        float timesNeededToWin = (float)(amountOwed)/(float)(payout);
        // if AI's call is not statistically worth it
        if (confidenceRatio < timesNeededToWin) {
            cout << "Daniel folds." << endl;
            return -1;
        }
        // if AI has reason to call
        else {
            // if this is just an all in decision
            if (amountOwed == AIStack) {
                cout << "Daniel calls, going all in for $" << AIStack << "!" << endl;
                return AIStack;
            } else if (userStack == 0) { // if the user just went all in
                cout << "Daniel calls, putting in $" << currBet-AILastBet << "." << endl;
                return currBet-AILastBet;
            }
            int rando = rand()%100;
            // calculate where AI is within range of calling/raising
            // if timesNeededToWin = 0.25, and confidenceRatio = 0.5, this would equal 0.33
            // (0.5-0.25)/(1-0.25) = 0.25/0.75 = 1/3
            float raiseVsCall = (confidenceRatio-timesNeededToWin)/(1-timesNeededToWin);
            // if on lower 0.25 of call/raise range
            if (raiseVsCall <= 0.25) {
                // always call
                cout << "Daniel calls, putting in $" << currBet-AILastBet << "." << endl;
                return currBet-AILastBet;
            }
            // if between 0.25 and 0.5 of call/raise range
            else if (raiseVsCall <= 0.50) {
                if (rando <= 75) { // call 3/4 of the time
                    cout << "Daniel calls, putting in $" << currBet-AILastBet << "." << endl;
                    return currBet-AILastBet;
                } else { // raise 1/4 of the time
                    int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
                    // Remove some hands here, because if the user just calls then the AI needs to update
                    removeHandsFromRange(currBet+bet, currBet, potSize+bet, AIStack, userStack, AIHand, boardCards, betRound, deck);
                    return currBet+bet-AILastBet;
                }
            }
            // if between 0.5 and 0.75 of call/raise range
            else if (raiseVsCall <= 0.75) {
                if (rando <= 25) { // call 1/4 of the time
                    cout << "Daniel calls, putting in $" << currBet-AILastBet << "." << endl;
                    return currBet-AILastBet;
                } else { // raise 3/4 of the time
                    int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
                    // Remove some hands here, because if the user just calls then the AI needs to update
                    removeHandsFromRange(currBet+bet, currBet, potSize+bet, AIStack, userStack, AIHand, boardCards, betRound, deck);
                    return currBet+bet-AILastBet;
                }
            }
            // if in top 0.25 of call/raise range
            else { // always raise
                int bet = determineBetSize(currBet, AILastBet, potSize, AIStack, userStack, confidenceRatio);
                // Remove some hands here, because if the user just calls then the AI needs to update
                removeHandsFromRange(currBet+bet, currBet, potSize+bet, AIStack, userStack, AIHand, boardCards, betRound, deck);
                return currBet+bet-AILastBet;
            }
        }
    }
    return 0.0;
}


// Function for removing hands from the user's possible range
// Depending on stage of hand, and the user's bet size compared to the pot, removes a variable number of hands
// "Removed" hands are made to contain references to the 53rd card of the deck (index 52)
// This "dead" card has a value of -1 and a suit of 'X'
float AI::removeHandsFromRange(int currBet, int AILastBet, int potSize, int AIStack, int userStack, Card* AIHand, Card* boardCards, int betRound, Card* deck) {
    
    /*
    for (int i = 0; i < 1326; i++) {
        cout << userRange[i][0] << ", " << userRange[i][1] << endl;
    }
     */
    
    // if beginning of hand, remove cards from userRange that contain AI's cards
    if (betRound == 0) {
        int numRemoved = 0;
        int x = 0;
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                // check first of AI's cards for presence in userRange (both slots of userRange)
                if ((deck[userRange[x][0]].suit == AIHand[0].suit) && (deck[userRange[x][0]].value == AIHand[0].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                } else if ((deck[userRange[x][1]].suit == AIHand[0].suit) && (deck[userRange[x][1]].value == AIHand[0].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                }
                // then check second of AI's cards for presence in userRange (both slots of userRange)
                else if ((deck[userRange[x][0]].suit == AIHand[1].suit) && (deck[userRange[x][0]].value == AIHand[1].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                } else if ((deck[userRange[x][1]].suit == AIHand[1].suit) && (deck[userRange[x][1]].value == AIHand[1].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                }
                x += 1;
            }
        }
        possibleUserHands -= numRemoved;
    }
    // on the flop, the AI can remove cards from the user's range that contain the three cards just dealt on the board
    else if (betRound == 1) {
        int numRemoved = 0;
        int x = 0;
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                // check first board card
                if ((deck[userRange[x][0]].suit == boardCards[0].suit) && (deck[userRange[x][0]].value == boardCards[0].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                } else if ((deck[userRange[x][1]].suit == boardCards[0].suit) && (deck[userRange[x][1]].value == boardCards[0].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                }
                // check second board card
                else if ((deck[userRange[x][0]].suit == boardCards[1].suit) && (deck[userRange[x][0]].value == boardCards[1].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                } else if ((deck[userRange[x][1]].suit == boardCards[1].suit) && (deck[userRange[x][1]].value == boardCards[1].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                }
                // check third board card
                else if ((deck[userRange[x][0]].suit == boardCards[2].suit) && (deck[userRange[x][0]].value == boardCards[2].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                } else if ((deck[userRange[x][1]].suit == boardCards[2].suit) && (deck[userRange[x][1]].value == boardCards[2].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                }
                x += 1;
            }
        }
        possibleUserHands -= numRemoved;
    }
    // on the turn, the AI can remove cards from the user's range that contain the fourth card just dealt on the board
    else if (betRound == 2) {
        int numRemoved = 0;
        int x = 0;
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                // check fourth board card
                if ((deck[userRange[x][0]].suit == boardCards[3].suit) && (deck[userRange[x][0]].value == boardCards[3].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                } else if ((deck[userRange[x][1]].suit == boardCards[3].suit) && (deck[userRange[x][1]].value == boardCards[3].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                }
                x += 1;
            }
        }
        possibleUserHands -= numRemoved;
    }
    // on the river, the AI can remove cards from the user's range that contain the fifth card just dealt on the board
    else if (betRound == 3) {
        int numRemoved = 0;
        int x = 0;
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                // check fifth board card
                if ((deck[userRange[x][0]].suit == boardCards[4].suit) && (deck[userRange[x][0]].value == boardCards[4].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                } else if ((deck[userRange[x][1]].suit == boardCards[4].suit) && (deck[userRange[x][1]].value == boardCards[4].value)) {
                    numRemoved += 1;
                    userRange[x][0] = 52;
                    userRange[x][1] = 52;
                }
                x += 1;
            }
        }
        possibleUserHands -= numRemoved;
    }
    
    // if user bets into AI, and AI owes amount > 1 ( it would only be 1 if AI is little blind, in which case no user hands should be elim
    if ((currBet - AILastBet) > 1) {
        // go through userRange
        float currStrengths[possibleUserHands];
        int j = 0;
        for (int i = 0; i < 1326; i++) {
            if (deck[userRange[i][0]].value >= 0) {
                currStrengths[j] = handStrengths[i];
                j += 1;
            }
        }
        // sort array (ends up being sorted highest -> lowest)
        std::sort(currStrengths, currStrengths + possibleUserHands, std::greater<float>());
        
        int amountOwed = currBet - AILastBet;
        float userBetPotRatio = (float)(amountOwed)/(float)(potSize - amountOwed);
        float ratioHandsRemoved = 0.0;
        if (userBetPotRatio <= 0.25) {
            ratioHandsRemoved = 0.25;
        } else if (userBetPotRatio <= 0.5) {
            ratioHandsRemoved = 0.40;
        } else if (userBetPotRatio <= 1.0) {
            ratioHandsRemoved = 0.50;
        } else if (userBetPotRatio <= 2.0) {
            ratioHandsRemoved = 0.70;
        } else if (userBetPotRatio <= 4.0) {
            ratioHandsRemoved = 0.80;
        } else {
            ratioHandsRemoved = 0.90;
        }
        
        int thresholdValueIndex = (int)((1.0-ratioHandsRemoved)*possibleUserHands);
        float thresholdValue = currStrengths[thresholdValueIndex];
        
        int numRemoved = 0;
        // now iterate through userRange
        for (int i = 0; i < 1326; i++) {
            // only look at hands that haven't already been removed
            if (deck[userRange[i][0]].value >= 0) {
                // if this hand's strength is under the threshold we determined
                if (handStrengths[i] < thresholdValue) {
                    handStrengths[i] = -1.5;
                    userRange[i][0] = 52;
                    userRange[i][1] = 52;
                    numRemoved += 1;
                }
            }
        }
        possibleUserHands -= numRemoved;
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
    float AIHandStrength = 0.0;
    // if pre-flop, hand strength determined just from flush, straight, and good pair odds
    if (betRound==0) {
        AIHandStrength = determineFlushOdds(playableCards, betRound) + determineStraightOdds(playableCards, betRound) + determineGoodPairOdds(playableCards, betRound);
    }
    // if flop, turn, or river, hand strength determined with all hand possibilities
    else {
        AIHandStrength = determineFlushOdds(playableCards, betRound) + determineStraightOdds(playableCards, betRound) +
            determineGoodPairOdds(playableCards, betRound) + determineStraightFlushOdds(playableCards, betRound) +
            determineFourOfAKindOdds(playableCards, betRound) + determineFullHouseOdds(playableCards, betRound) +
            determineThreeOfAKindOdds(playableCards, betRound) + determineTwoPairOdds(playableCards, betRound);
    }
    int numWorseHands = 0;
    int numTotalHands = 0;
    float confidenceRatio = 0.0;
    float userHandStrength = 0.0;
    int numChecked = 0;
    for (int i = 0; i < 1326; i += 1) { // go through all possible two card hands
        if (deck[userRange[i][0]].value != -1) { // all of the hands that AI has decided user could still have
            numChecked += 1;
            playableCards[0] = deck[userRange[i][0]];
            playableCards[1] = deck[userRange[i][1]];
            // determine how strong this hand is
            // if pre-flop, hand strength determined just from flush, straight, and good pair odds
            if (betRound==0) {
                userHandStrength = determineFlushOdds(playableCards, betRound) + determineStraightOdds(playableCards, betRound) + determineGoodPairOdds(playableCards, betRound);
            }
            // if flop, turn, or river, hand strength determined with all hand possibilities
            else {
                userHandStrength = determineFlushOdds(playableCards, betRound) + determineStraightOdds(playableCards, betRound) +
                determineGoodPairOdds(playableCards, betRound) + determineStraightFlushOdds(playableCards, betRound) +
                determineFourOfAKindOdds(playableCards, betRound) + determineFullHouseOdds(playableCards, betRound) +
                determineThreeOfAKindOdds(playableCards, betRound) + determineTwoPairOdds(playableCards, betRound);
            }
            handStrengths[i] = userHandStrength; // store this hand strength
            if (userHandStrength <= AIHandStrength) { // if AI has a better (or equal) hand, count it (num hands AI beats)
                numWorseHands += 1;
            }
            numTotalHands += 1; // count total hands user could have
        } else {
            handStrengths[i] = -1.5;
        }
    }
    confidenceRatio = (float)(numWorseHands)/(float)(numTotalHands);
    return confidenceRatio;
}



// Function for determining the AI's bet size (when the AI wants to raise).  This decision is based on how confident the AI
// is in its hand, the current bet, the pot size, etc.
// Returns an int referring to the amount the AI wishes to bet
int AI::determineBetSize(int currBet, int AILastBet, int potSize, int AIStack, int userStack, float confidenceRatio) {
    int rando = rand()%100;
    int bet;
    
    // if the AI is very confident
    if (confidenceRatio > 0.8) {
        // 25% of the time, bet small (about half pot)
        if (rando <= 25) {
            float newRando = (float)((rando / 50) - 0.25);
            bet = (int)((newRando * potSize) + 0.5);
        }
        // 75% of the time, bet size based on AI's (high) confidence
        else {
            bet = (int)(confidenceRatio * potSize);
        }
    }
    // if the AI not extremely confident (but still wants to raise)
    else {
        // 20% of the time, bet high (about pot size) -- bluff!
        if (rando <= 20) {
            float newRando = (float)(rando/60);
            bet = (int)((1-newRando) * potSize);
        }
        // 80% of the time, bet size based on AI's (not that high) confidence
        else {
            bet = (int)(confidenceRatio * potSize);
        }
    }
    
    
    
    
    // if bet smaller than allowed, make it minimum (big blind)
    if (bet < 2) {
        bet = 2;
    }
    // if bet is less than 2*currBet, make it 2*currBet
    if (bet < 2*currBet) {
        bet = 2*currBet;
    }
    
    // if AI's desired bet is more than it has, only allow bet of AIStack
    if (bet > AIStack) {
        bet = AIStack;
        cout << "Daniel puts in $" << bet << ", going all in!" << endl;
    } else {
        cout << "Daniel raises to $" << currBet+bet << "." << endl;
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
            return 5.0;
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
            return 5.0;
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
            return 5.0;
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
                    for (int k = 0; k < 5; k++) { // see if second card below exists
                        if (cards[k].value == cards[i].value - 2) {
                            if (bestVal < 0.4) {
                                bestVal = 0.4;
                            }
                            for (int l = 0; l < 5; l++) { // see if third card below exists
                                if (cards[l].value == cards[i].value - 3) {
                                    if (bestVal < 0.6) {
                                        bestVal = 0.6;
                                    }
                                    for (int m = 0; m < 5; m++) { // see if straight is already made
                                        if (cards[m].value == cards[i].value - 4) {
                                            if (bestVal < 4.0) {
                                                bestVal = 4.0;
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
        return bestVal;
    } else if (betRound == 2) { // turn
        float bestVal = 0.0;
        for (int i = 0; i < 6; i++) { // test each card as top card of straight
            for (int j = 0; j < 6; j++) { // see if first card below exists
                if (cards[j].value == cards[i].value - 1) {
                    for (int k = 0; k < 6; k++) { // see if second card below exists
                        if (cards[k].value == cards[i].value - 2) {
                            for (int l = 0; l < 6; l++) { // see if third card below exists
                                if (cards[l].value == cards[i].value - 3) {
                                    if (bestVal < 0.6) {
                                        bestVal = 0.6;
                                    }
                                    for (int m = 0; m < 6; m++) { // see if straight is already made
                                        if (cards[m].value == cards[i].value - 4) {
                                            if (bestVal < 4.0) {
                                                bestVal = 4.0;
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
        return bestVal;
    } else { // river
        float bestVal = 0.0;
        for (int i = 0; i < 7; i++) { // test each card as top card of straight
            for (int j = 0; j < 7; j++) { // see if first card below exists
                if (cards[j].value == cards[i].value - 1) {
                    for (int k = 0; k < 7; k++) { // see if second card below exists
                        if (cards[k].value == cards[i].value - 2) {
                            for (int l = 0; l < 7; l++) { // see if third cards below exists
                                if (cards[l].value == cards[i].value - 3) {
                                    for (int m = 0; m < 7; m++) { // see if straight is already made
                                        if (cards[m].value == cards[i].value - 4) {
                                            if (bestVal < 4.0) {
                                                bestVal = 4.0;
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
                return 5.0;
            } else if (cards[0].value == 13) { // and if pocket kings
                return 4.75;
            } else if (cards[0].value == 12) { // and if pocket queens
                return 4.5;
            } else {
                float val = (cards[0].value / 14.0) * 3.0;
                return val;
            }
        } else {
            float cardSum = cards[0].value + cards[1].value;
            cardSum = (float)(cardSum/27*1.5); // highest sum, of not paired cards, would be 14 + 13 (ace king) so we normalize across that range.  I then multiply 1.5 as a custom factor to get the AI to play more hands with high cards
            return cardSum;
        }
    } else if (betRound == 1) { // flop
        bool hasPair = false;
        int pairVal = -1;
        int highVal = -1;
        // find if pair exists, remember highest pair (and also remember highest val sans pair)
        for (int i = 0; i < 5; i++) {
            if (cards[i].value > highVal) {
                highVal = cards[i].value;
            }
            for (int j = 0; j < 5; j++) {
                if ((cards[i].value == cards[j]. value) && (i != j)) {
                    hasPair = true;
                    if (cards[i].value > pairVal) {
                        pairVal = cards[i].value;
                    }
                }
            }
        }
        if (hasPair) { // if a pair exists
            if (pairVal == 14) { // if aces
                return 2.5;
            } else if (pairVal == 13) { // if kings
                return 2.25;
            } else if (pairVal == 12) { // if queens
                return 2.0;
            } else { // anything lower
                float val = (float)((pairVal / 14.0)*1.5);
                return val;
            }
        } else {
            // if no pair, just return highVal, normalized across 1 according to all possible card values (2 - 14)
            float val = (float)(highVal / 14.0);
            return val;
        }
    } else if (betRound == 2) { // turn
        bool hasPair = false;
        int pairVal = -1;
        int highVal = -1;
        // find if pair exists, remember highest pair (and also remember highest val sans pair)
        for (int i = 0; i < 6; i++) {
            if (cards[i].value > highVal) {
                highVal = cards[i].value;
            }
            for (int j = 0; j < 6; j++) {
                if ((cards[i].value == cards[j]. value) && (i != j)) {
                    hasPair = true;
                    if (cards[i].value > pairVal) {
                        pairVal = cards[i].value;
                    }
                }
            }
        }
        if (hasPair) { // if a pair exists
            if (pairVal == 14) { // if aces
                return 2.5;
            } else if (pairVal == 13) { // if kings
                return 2.25;
            } else if (pairVal == 12) { // if queens
                return 2.0;
            } else { // anything lower
                float val = (float)((pairVal / 14.0)*1.5);
                return val;
            }
        } else {
            // if no pair, just return highVal, normalized across 1 according to all possible card values (2 - 14)
            float val = (float)(highVal / 14.0);
            return val;
        }
    } else { // river
        bool hasPair = false;
        int pairVal = -1;
        int highVal = -1;
        // find if pair exists, remember highest pair (and also remember highest val sans pair)
        for (int i = 0; i < 7; i++) {
            if (cards[i].value > highVal) {
                highVal = cards[i].value;
            }
            for (int j = 0; j < 7; j++) {
                if ((cards[i].value == cards[j]. value) && (i != j)) {
                    hasPair = true;
                    if (cards[i].value > pairVal) {
                        pairVal = cards[i].value;
                    }
                }
            }
        }
        if (hasPair) { // if a pair exists
            if (pairVal == 14) { // if aces
                return 2.5;
            } else if (pairVal == 13) { // if kings
                return 2.25;
            } else if (pairVal == 12) { // if queens
                return 2.0;
            } else { // anything lower
                float val = (float)((pairVal / 14.0)*1.5);
                return val;
            }
        } else {
            // if no pair, just return highVal, normalized across 1 according to all possible card values (2 - 14)
            float val = (float)(highVal / 14.0);
            return val;
        }
    }
    return 0.0;
}

// Determines how likely it is for the input cards to achieve a straight flush
float AI::determineStraightFlushOdds(Card* cards, int betRound) {
    // flop
    if (betRound == 1) {
        float bestVal = 0.0;
        for (int i = 0; i < 5; i++) { // test each card as top card of straight flush
            for (int j = 0; j < 5; j++) { // see if first card below exists
                if ((cards[j].value == cards[i].value - 1) && (cards[j].suit == cards[i].suit)) {
                    for (int k = 0; k < 5; k++) { // see if second card below exists
                        if ((cards[k].value == cards[i].value - 2) && (cards[k].suit == cards[i].suit)) {
                            if (bestVal < 0.4) {
                                bestVal = 0.4;
                            }
                            for (int l = 0; l < 5; l++) { // see if third cards below exists
                                if ((cards[l].value == cards[i].value - 3) && (cards[l].suit == cards[i].suit)) {
                                    if (bestVal < 0.7) {
                                        bestVal = 0.7;
                                    }
                                    for (int m = 0; m < 5; m++) { // see if straight flush is already made
                                        if ((cards[m].value == cards[i].value - 4) && (cards[m].suit == cards[i].suit)) {
                                            if (bestVal < 7.0) {
                                                bestVal = 7.0;
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
        return bestVal;
    }
    // turn
    else if (betRound == 2) {
        float bestVal = 0.0;
        for (int i = 0; i < 6; i++) { // test each card as top card of straight flush
            for (int j = 0; j < 6; j++) { // see if first card below exists
                if ((cards[j].value == cards[i].value - 1) && (cards[j].suit == cards[i].suit)) {
                    for (int k = 0; k < 6; k++) { // see if second card below exists
                        if ((cards[k].value == cards[i].value - 2) && (cards[k].suit == cards[i].suit)) {
                            for (int l = 0; l < 6; l++) { // see if third cards below exists
                                if ((cards[l].value == cards[i].value - 3) && (cards[l].suit == cards[i].suit)) {
                                    if (bestVal < 0.6) {
                                        bestVal = 0.6;
                                    }
                                    for (int m = 0; m < 6; m++) { // see if straight flush is already made
                                        if ((cards[m].value == cards[i].value - 4) && (cards[m].suit == cards[i].suit)) {
                                            if (bestVal < 7.0) {
                                                bestVal = 7.0;
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
        return bestVal;
    }
    // river
    else {
        float bestVal = 0.0;
        for (int i = 0; i < 7; i++) { // test each card as top card of straight flush
            for (int j = 0; j < 7; j++) { // see if first card below exists
                if ((cards[j].value == cards[i].value - 1) && (cards[j].suit == cards[i].suit)) {
                    for (int k = 0; k < 7; k++) { // see if second card below exists
                        if ((cards[k].value == cards[i].value - 2) && (cards[k].suit == cards[i].suit)) {
                            for (int l = 0; l < 7; l++) { // see if third cards below exists
                                if ((cards[l].value == cards[i].value - 3) && (cards[l].suit == cards[i].suit)) {
                                    for (int m = 0; m < 7; m++) { // see if straight flush is already made
                                        if ((cards[m].value == cards[i].value - 4) && (cards[m].suit == cards[i].suit)) {
                                            if (bestVal < 7.0) {
                                                bestVal = 7.0;
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
        return bestVal;
    }
    return 0.0;
}

// Determines how likely it is for the input cards to achieve a four of a kind
float AI::determineFourOfAKindOdds(Card* cards, int betRound) {
    // flop
    if (betRound == 1) {
        int numSame = 1;
        for (int i = 0; i < 5; i++) { // test each card as base of four of a kind
            for (int j = 0; j < 5; j++) { // look for second card
                if ((cards[j].value == cards[i].value) && (j != i)) {
                    // if another card of same value in hand, but not same as first
                    if (numSame < 2) {
                        numSame = 2;
                    }
                    for (int k = 0; k < 5; k++) { // look for third card
                        if ((cards[k].value == cards[i].value) && (k != i) && (k != j)) {
                            // if another card of same value in hand, but not same as first/second
                            if (numSame < 3) {
                                numSame = 3;
                            }
                            for (int l = 0; l < 5; l++) { // look for fourth card
                                if ((cards[l].value == cards[i].value) && (l != i) && (l != j) && (l != k)) {
                                    // if another card of same value in hand, but not same as first/second/third
                                    if (numSame < 4) {
                                        numSame = 4;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (numSame == 4) { // if made four of a kind
            return 6.5;
        } else if (numSame == 3) { // if one card away
            return 0.7;
        } else if (numSame == 2) { // if two away
            return 0.4;
        } else { // else four of a kind not possible
            return 0.0;
        }
    }
    // turn
    else if (betRound == 2) {
        int numSame = 1;
        for (int i = 0; i < 6; i++) { // test each card as base of four of a kind
            for (int j = 0; j < 6; j++) { // look for second card
                if ((cards[j].value == cards[i].value) && (j != i)) {
                    // if another card of same value in hand, but not same as first
                    if (numSame < 2) {
                        numSame = 2;
                    }
                    for (int k = 0; k < 6; k++) { // look for third card
                        if ((cards[k].value == cards[i].value) && (k != i) && (k != j)) {
                            // if another card of same value in hand, but not same as first/second
                            if (numSame < 3) {
                                numSame = 3;
                            }
                            for (int l = 0; l < 6; l++) { // look for fourth card
                                if ((cards[l].value == cards[i].value) && (l != i) && (l != j) && (l != k)) {
                                    // if another card of same value in hand, but not same as first/second/third
                                    if (numSame < 4) {
                                        numSame = 4;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (numSame == 4) { // if made four of a kind
            return 6.5;
        } else if (numSame == 3) { // if one card away
            return 0.6;
        } else { // else four of a kind not possible
            return 0.0;
        }
    }
    // river
    else {
        int numSame = 1;
        for (int i = 0; i < 7; i++) { // test each card as base of four of a kind
            for (int j = 0; j < 7; j++) { // look for second card
                if ((cards[j].value == cards[i].value) && (j != i)) {
                    // if another card of same value in hand, but not same as first
                    if (numSame < 2) {
                        numSame = 2;
                    }
                    for (int k = 0; k < 7; k++) { // look for third card
                        if ((cards[k].value == cards[i].value) && (k != i) && (k != j)) {
                            // if another card of same value in hand, but not same as first/second
                            if (numSame < 3) {
                                numSame = 3;
                            }
                            for (int l = 0; l < 7; l++) { // look for fourth card
                                if ((cards[l].value == cards[i].value) && (l != i) && (l != j) && (l != k)) {
                                    // if another card of same value in hand, but not same as first/second/third
                                    if (numSame < 4) {
                                        numSame = 4;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (numSame == 4) { // if made four of a kind
            return 6.5;
        } else { // else four of a kind not possible
            return 0.0;
        }
    }
    return 0.0;
}

// Determines how likely it is for the input cards to achieve a full house
float AI::determineFullHouseOdds(Card* cards, int betRound) {
    // flop
    if (betRound == 1) {
        // first find most common val
        int mostCommonVal = -1;
        int mostFreq = 1;
        for (int i = 0; i < 5; i++) {
            int freq = 1;
            for (int j = 0; j < 5; j++) {
                if ((cards[i].value == cards[j].value) && (i != j)) {
                    freq += 1;
                }
            }
            if (freq > mostFreq) {
                mostFreq = freq;
                mostCommonVal = cards[i].value;
            }
        }
        // then find second most freq
        int secondMostCommonVal = -1;
        int secondMostFreq = 1;
        for (int i = 0; i < 5; i++) {
            if (cards[i].value != mostCommonVal) {
                int freq = 1;
                for (int j = 0; j < 5; j++) {
                    if ((cards[i].value == cards[j].value) && (i != j)) {
                        freq += 1;
                    }
                }
                if (freq > secondMostFreq) {
                    secondMostFreq = freq;
                    secondMostCommonVal = cards[i].value;
                }
            }
        }
        if (mostFreq >= 3) {
            if (secondMostFreq >= 2) { // made full house
                return 6.0;
            } else { // have 3 and 1
                return 0.7;
            }
        } else if (mostFreq == 2) {
            if (secondMostFreq == 2) { // have 2 and 2
                return 1.0;
            } else { // have 2 and 1
                return 0.4;
            }
        } else { // 1 and 1 (nothing)
            return 0.0;
        }
    }
    // turn
    else if (betRound == 2) {
        // first find most common val
        int mostCommonVal = -1;
        int mostFreq = 1;
        for (int i = 0; i < 6; i++) {
            int freq = 1;
            for (int j = 0; j < 6; j++) {
                if ((cards[i].value == cards[j].value) && (i != j)) {
                    freq += 1;
                }
            }
            if (freq > mostFreq) {
                mostFreq = freq;
                mostCommonVal = cards[i].value;
            }
        }
        // then find second most freq
        int secondMostCommonVal = -1;
        int secondMostFreq = 1;
        for (int i = 0; i < 6; i++) {
            if (cards[i].value != mostCommonVal) {
                int freq = 1;
                for (int j = 0; j < 6; j++) {
                    if ((cards[i].value == cards[j].value) && (i != j)) {
                        freq += 1;
                    }
                }
                if (freq > secondMostFreq) {
                    secondMostFreq = freq;
                    secondMostCommonVal = cards[i].value;
                }
            }
        }
        if (mostFreq >= 3) {
            if (secondMostFreq >= 2) { // made full house
                return 6.0;
            } else { // have 3 and 1
                return 0.6;
            }
        } else if (mostFreq == 2) {
            if (secondMostFreq == 2) { // have 2 and 2
                return 0.9;
            } else { // have 2 and 1
                return 0.0;
            }
        } else { // 1 and 1 (nothing)
            return 0.0;
        }
    }
    // river
    else {
        // first find most common val
        int mostCommonVal = -1;
        int mostFreq = 1;
        for (int i = 0; i < 7; i++) {
            int freq = 1;
            for (int j = 0; j < 7; j++) {
                if ((cards[i].value == cards[j].value) && (i != j)) {
                    freq += 1;
                }
            }
            if (freq > mostFreq) {
                mostFreq = freq;
                mostCommonVal = cards[i].value;
            }
        }
        // then find second most freq
        int secondMostCommonVal = -1;
        int secondMostFreq = 1;
        for (int i = 0; i < 7; i++) {
            if (cards[i].value != mostCommonVal) {
                int freq = 1;
                for (int j = 0; j < 7; j++) {
                    if ((cards[i].value == cards[j].value) && (i != j)) {
                        freq += 1;
                    }
                }
                if (freq > secondMostFreq) {
                    secondMostFreq = freq;
                    secondMostCommonVal = cards[i].value;
                }
            }
        }
        if (mostFreq >= 3) {
            if (secondMostFreq >= 2) { // made full house
                return 6.0;
            } else { // have 3 and 1
                return 0.0;
            }
        } else if (mostFreq == 2) {
            if (secondMostFreq == 2) { // have 2 and 2
                return 0.0;
            } else { // have 2 and 1
                return 0.0;
            }
        } else { // 1 and 1 (nothing)
            return 0.0;
        }
    }
    return 0.0;
}

// FLUSH DETERMINATION ABOVE

// STRAIGHT DETERMINATION ABOVE

// Determines how likely it is for the input cards to achieve a three of a kind
float AI::determineThreeOfAKindOdds(Card* cards, int betRound) {
    // flop
    if (betRound == 1) {
        int numSame = 1;
        for (int i = 0; i < 5; i++) { // test each card as base of three of a kind
            for (int j = 0; j < 5; j++) { // look for second card
                if ((cards[j].value == cards[i].value) && (j != i)) {
                    // if another card of same value in hand, but not same as first
                    if (numSame < 2) {
                        numSame = 2;
                    }
                    for (int k = 0; k < 5; k++) { // look for third card
                        if ((cards[k].value == cards[i].value) && (k != i) && (k != j)) {
                            // if another card of same value in hand, but not same as first/second
                            if (numSame < 3) {
                                numSame = 3;
                            }
                        }
                    }
                }
            }
        }
        if (numSame == 3) { // if made three of a kind
            return 3.5;
        } else if (numSame == 2) { // if one away
            return 1.0;
        } else { // if no pairs on the flop
            return 0.5;
        }
    }
    // turn
    else if (betRound == 2) {
        int numSame = 1;
        for (int i = 0; i < 6; i++) { // test each card as base of four of a kind
            for (int j = 0; j < 6; j++) { // look for second card
                if ((cards[j].value == cards[i].value) && (j != i)) {
                    // if another card of same value in hand, but not same as first
                    if (numSame < 2) {
                        numSame = 2;
                    }
                    for (int k = 0; k < 6; k++) { // look for third card
                        if ((cards[k].value == cards[i].value) && (k != i) && (k != j)) {
                            // if another card of same value in hand, but not same as first/second
                            if (numSame < 3) {
                                numSame = 3;
                            }
                        }
                    }
                }
            }
        }
        if (numSame == 3) { // if made three of a kind
            return 3.5;
        } else if (numSame == 2) { // if one card away
            return 1.0;
        } else { // three of a kind not possible
            return 0.0;
        }
    }
    // river
    else {
        int numSame = 1;
        for (int i = 0; i < 7; i++) { // test each card as base of four of a kind
            for (int j = 0; j < 7; j++) { // look for second card
                if ((cards[j].value == cards[i].value) && (j != i)) {
                    // if another card of same value in hand, but not same as first
                    if (numSame < 2) {
                        numSame = 2;
                    }
                    for (int k = 0; k < 7; k++) { // look for third card
                        if ((cards[k].value == cards[i].value) && (k != i) && (k != j)) {
                            // if another card of same value in hand, but not same as first/second
                            if (numSame < 3) {
                                numSame = 3;
                            }
                        }
                    }
                }
            }
        }
        if (numSame == 3) { // if made three of a kind
            return 3.5;
        } else { // three of a kind not possible
            return 0.0;
        }
    }
    return 0.0;
}

// Determines how likely it is for the input cards to achieve a two pair
float AI::determineTwoPairOdds(Card* cards, int betRound) {
    // flop
    if (betRound == 1) {
        // first look for first pair
        int firstVal = -1;
        int firstFreq = 1;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if ((cards[j].value == cards[i].value) && (i != j)) {
                    // if same value exists, and not same card as first
                    firstVal = cards[i].value;
                    firstFreq = 2;
                }
            }
        }
        // then look for second pair
        int secondVal = -1;
        int secondFreq = 1;
        for (int i = 0; i < 5; i++) {
            if (cards[i].value != firstVal) {
                // look for second pair, but ignore card if same value as first pair found
                for (int j = 0; j < 5; j++) {
                    if ((cards[j].value == cards[i].value) && (i != j)) {
                        // if same value exists, and not same card as first
                        secondVal = cards[i].value;
                        secondFreq = 2;
                    }
                }
            }
        }
        if (firstFreq == 2) {
            if (secondFreq == 2) { // two pair made
                return 2.75;
            } else { // 2, 1
                return 0.7;
            }
        } else { // 1, 1
            return 0.3;
        }
    }
    // turn
    else if (betRound == 2) {
        // first look for first pair
        int firstVal = -1;
        int firstFreq = 1;
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                if ((cards[j].value == cards[i].value) && (i != j)) {
                    // if same value exists, and not same card as first
                    firstVal = cards[i].value;
                    firstFreq = 2;
                }
            }
        }
        // then look for second pair
        int secondVal = -1;
        int secondFreq = 1;
        for (int i = 0; i < 6; i++) {
            if (cards[i].value != firstVal) {
                // look for second pair, but ignore card if same value as first pair found
                for (int j = 0; j < 6; j++) {
                    if ((cards[j].value == cards[i].value) && (i != j)) {
                        // if same value exists, and not same card as first
                        secondVal = cards[i].value;
                        secondFreq = 2;
                    }
                }
            }
        }
        if (firstFreq == 2) {
            if (secondFreq == 2) { // two pair made
                return 2.75;
            } else { // 2, 1
                return 0.6;
            }
        } else { // 1, 1
            return 0.0;
        }
    }
    // river
    else {
        // first look for first pair
        int firstVal = -1;
        int firstFreq = 1;
        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 7; j++) {
                if ((cards[j].value == cards[i].value) && (i != j)) {
                    // if same value exists, and not same card as first
                    firstVal = cards[i].value;
                    firstFreq = 2;
                }
            }
        }
        // then look for second pair
        int secondVal = -1;
        int secondFreq = 1;
        for (int i = 0; i < 7; i++) {
            if (cards[i].value != firstVal) {
                // look for second pair, but ignore card if same value as first pair found
                for (int j = 0; j < 7; j++) {
                    if ((cards[j].value == cards[i].value) && (i != j)) {
                        // if same value exists, and not same card as first
                        secondVal = cards[i].value;
                        secondFreq = 2;
                    }
                }
            }
        }
        if (firstFreq == 2) {
            if (secondFreq == 2) { // two pair made
                return 2.75;
            } else { // 2, 1
                return 0.0;
            }
        } else { // 1, 1
            return 0.0;
        }
    }
    return 0.0;
}

// PAIR DETERMINATION ABOVE




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
    possibleUserHands = 1326;
}
