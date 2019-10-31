//
//  Card.cpp
//  Texas Hold 'em
//
//  Created by Jonathan Redwine on 10/9/19.
//  Copyright © 2019 JonathanRedwine. All rights reserved.
//


class Card {
public:
    int value;
    char suit;
    Card() {
        value = 0;
        suit = 'X';
    }
    Card(int v, char s) {
        value = v;
        suit = s;
    }
};
