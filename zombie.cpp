#include <cmath>
#include <GL/glx.h>
#include <ctime>
#include "zombie.h"
#include "globals.h"
#include "player.h"

extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);

extern Player player;

Zombie::Zombie() {
    pos[0] = 0.0f;        // x
    pos[1] = 0.0f;        // y
    pos[2] = 0.0f;     // this position doesn't matter
    w = 40.0f;
    h = 40.0f;
    speed = 0.94;        // might have to change this (original: 1.32)
    angle = 0.0f;
    color[0] = 1.0f;  
    color[1] = 0.0f;    
    color[2] = 0.0f;
    health = 100;         // health for each zombie
    alive = false;
    spawnTimer = {0, 0};
}
void Zombie::init() {
    health = 100.0f;        // remove this line when i am spawning in multiple zombies
    alive = false;
    spawnTimer = {0, 0};
    clock_gettime(CLOCK_REALTIME, &spawnTimer);

    float minDist = 300.0f;  // makes sure to spawn the zombie not to close to the player
    
    // get them to spawn randomly outside somewhat far from the player
    float x, y;
    do {   
        x = (float)(rand() % gl.xres);
        y = (float)(rand() % gl.yres);

        float dx = x - player.pos[0];
        float dy = y - player.pos[1];
        float dist = sqrt (dx*dx + dy*dy);

        if (dist >= minDist) break;

    } while (true); 
    
    // set the random position to the zombie
    pos[0] = x;
    pos[1] = y;
}

void Zombie::update() {
    // wait a moment to spawn the zombie so that it's not instantly 
    if (!alive) {
        if (health <= 0.0f) return; // dead, dont revive zombie
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        double elapsed = timeDiff(&spawnTimer, &now);
        if (elapsed >= 0.43) {
            alive = true;
        }
        return;
    }


    // zombies should just move to to position of where the player is currently at
    float dx = player.pos[0] - pos[0];
    float dy = player.pos[1] - pos[1];

    float dist = sqrt(dx*dx + dy*dy);

    if (dist > 0.0f) {
        dx /= dist;
        dy /= dist;
    }

    pos[0] += dx * speed;
    pos[1] += dy * speed;

    angle = atan2(dy, dx);
}

void Zombie::render() {
    if (!alive) return;

    glColor3fv(color);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);

    // Draw Zombie
    glBegin(GL_LINE_LOOP);          // change this to an outline to kinda make it like a hitbox
        glVertex2f(-w / 2.0f, -h / 2.0f);
        glVertex2f(-w / 2.0f,  h / 2.0f);
        glVertex2f( w / 2.0f,  h / 2.0f);
        glVertex2f( w / 2.0f, -h / 2.0f);
    glEnd();
    glPopMatrix();
}
