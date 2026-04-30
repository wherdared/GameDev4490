#ifndef PLAYER_H
#define PLAYER_H

#include "globals.h"

class Player {
public:
    Vec pos;
    float w, h;
    float speed;
    float angle;
    float health;
    float maxHealth;
    float color[3];

    int damageCooldown;
    int flashTimer;

    Player();

    void update();
    void render();
    void takeDamage(float amount);
    void renderHealthBar();
};

#endif
