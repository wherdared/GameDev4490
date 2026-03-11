#ifndef PLAYER_H
#define PLAYER_H

#include "globals.h"

class Player {
public:
    Vec pos;
    float w, h;
    float speed;
    float angle;
    float color[3];

    Player();

    void update();
    void render();
};

#endif
