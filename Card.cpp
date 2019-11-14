//
//  Card.cpp
//  Texas Hold 'em
//
//  Created by Jonathan Redwine on 10/9/19.
//  Copyright © 2019 JonathanRedwine. All rights reserved.
//


// Card class
class Card {
public:
    // Cards just have two attributes - value (e.g., 3, 8, Jack(11), Ace(14), and suit (e.g., Hearts (H))
    int value;
    char suit;
    // Default constructor, creates a 'dead' card (value 0, suit X)
    Card() {
        value = 0;
        suit = 'X';
    }
    // Overload constructor, assigns attributes according to inputs
    Card(int v, char s) {
        value = v;
        suit = s;
    }
};
