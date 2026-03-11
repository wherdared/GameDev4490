#ifndef BULLET_H
#define BULLET_H

#include <ctime>
#include "globals.h"
#include "player.h"

const int MAX_BULLETS = 200;

class Bullet {
public:
    Vec pos;
    Vec vel;
    bool active;
    float color[3];

    Bullet();
};

class BulletManager {
public:
    Bullet bullets[MAX_BULLETS];
    int nbullets;
    bool shootingEnabled;
    double fireDelay;
    struct timespec bulletTimer;

    BulletManager();

    void spawn(const Player &player);
    void update(const Player &player);
    void render();
};

#endif
