//
//  main.cpp
//  Texas Hold 'em
//
//  Created by Jonathan Redwine on 10/9/19.
//  Copyright © 2019 JonathanRedwine. All rights reserved.
//

#include <iostream>
#include "GameManager.cpp"
using namespace std;


// Main function for the Texas Hold 'em app
int main(int argc, const char * argv[]) {
    srand((unsigned int)time(0)); // set seed for psuedo RNG, based on current time
    GameManager game;
    AI ai;
    cout << "Welcome to Texas Hold 'em!" << endl;
    cout << "You will be playing against an AI named Daniel Negreanu." << endl;
    cout << "The game's small and big blinds are $1 and $2.  Both you and Daniel begin with $200." << endl;
    int hand = 0; // track how many hands are played --> hand is odd means user deals, hand is even means AI deals
    int keepPlaying = 1;
    cout << endl << "Press any key to continue." << endl;
    while (cin.get() != '\n');
    /*
    Card* userTestHand = new Card[7];
    userTestHand[0].value = 14;
    userTestHand[0].suit = 'H';
    userTestHand[1].value = 6;
    userTestHand[1].suit = 'S';
    userTestHand[2].value = 2;
    userTestHand[2].suit = 'S';
    userTestHand[3].value = 3;
    userTestHand[3].suit = 'S';
    userTestHand[4].value = 5;
    userTestHand[4].suit = 'S';
    userTestHand[5].value = 4;
    userTestHand[5].suit = 'S';
    userTestHand[6].value = 14;
    userTestHand[6].suit = 'S';
    cout << "USER TEST HAND: ";
    int testUserStrength = game.findBestHand(userTestHand);
    
    Card* AITestHand = new Card[7];
    AITestHand[0].value = 13;
    AITestHand[0].suit = 'H';
    AITestHand[1].value = 7;
    AITestHand[1].suit = 'D';
    AITestHand[2].value = 2;
    AITestHand[2].suit = 'S';
    AITestHand[3].value = 3;
    AITestHand[3].suit = 'S';
    AITestHand[4].value = 5;
    AITestHand[4].suit = 'S';
    AITestHand[5].value = 4;
    AITestHand[5].suit = 'S';
    AITestHand[6].value = 14;
    AITestHand[6].suit = 'S';
    cout << "AI TEST HAND: ";
    int testAIStrength = game.findBestHand(AITestHand);
    game.resolveTie(testUserStrength, userTestHand, AITestHand);
    */
    
    // Keep playing hands until the user decides they wish to quit (or an invalid input is entered)
    while (keepPlaying == 1) {
        hand += 1;
        game.shuffleDeck();
        if ((hand%2) == 1) { // if user is dealer
            cout << "You are the dealer!" << endl;
            game.AIHand[0] = game.drawCard();
            game.userHand[0] = game.drawCard();
            game.AIHand[1] = game.drawCard();
            game.userHand[1] = game.drawCard();
            game.potSize = 3;
            game.userStack -= 2;
            game.AIStack -= 1;
        } else { // if AI is dealer
            cout << "Daniel is the dealer!" << endl;
            game.userHand[0] = game.drawCard();
            game.AIHand[0] = game.drawCard();
            game.userHand[1] = game.drawCard();
            game.AIHand[1] = game.drawCard();
            game.potSize = 3;
            game.userStack -= 1;
            game.AIStack -=2;
        }
        cout << "Dealing" << endl;
        usleep(500000);
        cout << "." << endl;
        usleep(500000);
        cout << "." << endl;
        usleep(500000);
        cout << "." << endl;
        usleep(500000);
        game.displayTable();
        // PRE-FLOP
        if (game.bettingRound(ai, hand%2, 0) != -1) { // if no fold happened pre-flop, proceed
            cout << "Dealing the flop" << endl;
            usleep(500000);
            cout << "." << endl;
            usleep(500000);
            cout << "." << endl;
            usleep(500000);
            cout << "." << endl;
            usleep(500000);
            game.drawCard();
            game.drawCard();
            game.drawCard();
            game.displayTable();
            // FLOP
            if (game.bettingRound(ai, hand%2, 1) != -1) { // if no fold happened at the flop, proceed
                cout << "Dealing the turn" << endl;
                usleep(500000);
                cout << "." << endl;
                usleep(500000);
                cout << "." << endl;
                usleep(500000);
                cout << "." << endl;
                usleep(500000);
                game.drawCard();
                game.displayTable();
                // TURN
                if (game.bettingRound(ai, hand%2, 1) != -1) { // if no fold happened at the turn, proceed
                    cout << "Dealing the river" << endl;
                    usleep(500000);
                    cout << "." << endl;
                    usleep(500000);
                    cout << "." << endl;
                    usleep(500000);
                    cout << "." << endl;
                    usleep(500000);
                    game.drawCard();
                    game.displayTable();
                    // RIVER
                    if (game.bettingRound(ai, hand%2, 1) != -1) { // if no fold happened at the river, showdown
                        cout << endl << "Daniel's hand: ";
                        for (int i = 0; i < 2; i++) {
                            if (game.AIHand[i].value == 10) {
                                cout << "T";
                            } else if (game.AIHand[i].value == 11) {
                                cout << "J";
                            } else if (game.AIHand[i].value == 12) {
                                cout << "Q";
                            } else if (game.AIHand[i].value == 13) {
                                cout << "K";
                            } else if (game.AIHand[i].value == 14) {
                                cout << "A";
                            } else {
                                cout << game.AIHand[i].value;
                            }
                            cout << game.AIHand[i].suit << " ";
                        }
                        cout << endl << "Daniel has: ";
                        Card* AIPlayableCards = new Card[7];
                        for (int i = 0; i < 5; i++) {
                            AIPlayableCards[i] = game.drawnCards[i+4];
                        }
                        for(int i = 0; i < 2; i++) {
                            AIPlayableCards[i+5] = game.AIHand[i];
                        }
                        int AIHandStrength = game.findBestHand(AIPlayableCards);
                        cout << endl << "You have: ";
                        Card* userPlayableCards = new Card[7];
                        for (int i = 0; i < 5; i++) {
                            userPlayableCards[i] = game.drawnCards[i+4];
                        }
                        for(int i = 0; i < 2; i++) {
                            userPlayableCards[i+5] = game.userHand[i];
                        }
                        int userHandStrength = game.findBestHand(userPlayableCards);
                        cout << endl;
                        int handWinner = -1;
                        if (userHandStrength > AIHandStrength) { // if user's hand is stronger
                            cout << "You win!" << endl;
                            handWinner = 1;
                        } else if (userHandStrength < AIHandStrength) { // if AI's hand is stronger
                            cout << "Daniel wins!" << endl;
                            handWinner = 2;
                        } else { // if they have the same strength hand, resolve tie
                            handWinner = game.resolveTie(userHandStrength, userPlayableCards, AIPlayableCards);
                        }
                        game.finishHand(handWinner, hand%2);
                    }
                }
            }
        }
        // Ask if user wishes to play another hand
        cout << endl << "Would you like to play another hand?" << endl << "1 - Keep playing" << endl << "0 - Quit" << endl;
        cin >> keepPlaying;
        if (!cin.eof()  && cin.good()) {
        }
    }
    // If game over, app exits
    cout << "Thank you for playing.  You finished with $" << game.userStack << "." << endl;
}




