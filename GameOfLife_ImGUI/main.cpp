//
// Created by
//          Benjamin Grothe
// on 06.05.23
// all Copyrights included by Benjamin Grothe
//

#include <iostream>
#include "include/game_of_life.h"

int main() {
    // START RULES
    gol::Rule r = (gol::Rule){{false,false, false, true, false, false, false, false, false},
                              {true, true, false, false, true, true, true, true, true}};

    // START COLOR RULES
    gol::RuleColor rc0 = (gol::RuleColor){
            IM_COL32(255,0,0,255),
            IM_COL32(198,57,0,255),
            IM_COL32(141,114,0,255),
            IM_COL32(84,171,0,255),
            IM_COL32(0,255,0,255),
            IM_COL32(0,198,57,255),
            IM_COL32(0,141,114,255),
            IM_COL32(0,84,171,255),
            IM_COL32(0,0,255,255)
    }; // Rainbow
/*
    gol::RuleColor rc1 = (gol::RuleColor){
            IM_COL32(0,   255, 0,      255),
            IM_COL32(0,   198, 57,     255),
            IM_COL32(0,   141, 114,    255),
            IM_COL32(0,   84,  171,    255),
            IM_COL32(0,   0,   255,    255),
            IM_COL32(57,  0,   198,    255),
            IM_COL32(114, 0,   141,    255),
            IM_COL32(171, 0,   84,     255),
            IM_COL32(255, 0,   0,      255)
    }; // Rainbow

    gol::RuleColor rc2 = (gol::RuleColor){
            IM_COL32(0,     0,   255,     255),
            IM_COL32(57,    0,   198,     255),
            IM_COL32(114,   0,   141,     255),
            IM_COL32(171,   0,   84,      255),
            IM_COL32(255,   0,   0,       255),
            IM_COL32(198,   57,  0,       255),
            IM_COL32(141,   114, 0,       255),
            IM_COL32(84,    171, 0,       255),
            IM_COL32(0,     255, 0,       255)
    }; // Rainbow

    gol::RuleColor rc3 = (gol::RuleColor){
            IM_COL32(255,   255,    255,  255),
            IM_COL32(255,   255,    255,  255),
            IM_COL32(255,   255,    255,  255),
            IM_COL32(255,   255,    255,  255),
            IM_COL32(255,   255,    255,  255),
            IM_COL32(255,   255,    255,  255),
            IM_COL32(255,   255,    255,  255),
            IM_COL32(255,   255,    255,  255),
            IM_COL32(255,   255,    255,  255)
    }; // White
    */

    gol::GameOfLife* GOL = new gol::GameOfLife((gol::Vec2){533,325}, r, rc0);
    //gol::GameOfLife* GOL = new gol::GameOfLife((gol::Vec2){700,500}, r, rc0);

    GOL->runUI();

    return 0;
}
