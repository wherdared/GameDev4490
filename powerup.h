#ifndef POWERUP_H
#define POWERUP_H

#include "globals.h"

class PowerUp {
    public:
        Vec pos;
        bool active;
        float radius;           
        float w, h;          
        float color[3];
        float explosionTimer;   
        float maxExplosionTime;
        float pulse;           

        PowerUp();
        void spawn(float x, float y);
        void update();
        void render();
};

extern PowerUp nukePowerUp;

#endif
