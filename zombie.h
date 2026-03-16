#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "globals.h"

class Zombie {
    public:
        Vec pos;
        float w, h;
        float speed;        // should be slower than player
        float angle;
        float color[3];     // show a color for rn different than the player
        float health;
        bool alive;
        struct timespec spawnTimer;

        Zombie();

        void init();
        void update();
        void render();
};

#endif
