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

        Zombie();

        void init();
        void update();
        void render();
};

#endif
