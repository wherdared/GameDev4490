#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "globals.h"
#include "sprite.h"

//const int MAX_ZOMBIES = 25;     // max amount of zombies that can be alive at a time not per round

class Zombie {
    public:
        Vec pos;
        float w, h;
        float speed;        // should be slower than player
        float angle;
        float color[3];     // show a color for rn different than the player
        float health;
        bool alive;
        bool wasHit;        // when true it will display health bar
        struct timespec spawnTimer;
        float frameTimer;
        int currentFrame;
        float hitFlashTimer;        // zombie flashes red indicating it has been hit

        Sprite *currentSprite;

        Zombie();

        void init();
        void update();
        void render();
};

#endif
