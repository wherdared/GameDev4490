#ifndef PLAYER_H
#define PLAYER_H

#include "globals.h"
#include "sprite.h"

class Player {
public:
    Vec pos;
    float w, h;
    float speed;
    float angle;
    float health;
    float maxHealth;
    float color[3];

    bool wasDamaged;
    int damageCooldown;
    int flashTimer;
    
    Sprite *currentSprite;
    
    Player();

    void update();
    void render();
    void takeDamage(float amount);
    void renderHealthBar();
};

#endif
