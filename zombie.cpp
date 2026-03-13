#include <cmath>
#include <GL/glx.h>
#include "zombie.h"
#include "globals.h"
#include "player.h"

extern Player player;

Zombie::Zombie() {
    pos[0] = 0.0f;        // x
    pos[1] = 0.0f;        // y
    pos[2] = 0.0f;     // this position doesn't matter
    w = 40.0f;
    h = 40.0f;
    speed = 1.95;        // might have to change this
    angle = 0.0f;
    color[0] = 1.0f;  
    color[1] = 0.0f;    
    color[2] = 0.0f;
    
}
void Zombie::init() {
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
    glColor3fv(color);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);

    // Draw Zombie
    glBegin(GL_QUADS);
        glVertex2f(-w / 2.0f, -h / 2.0f);
        glVertex2f(-w / 2.0f,  h / 2.0f);
        glVertex2f( w / 2.0f,  h / 2.0f);
        glVertex2f( w / 2.0f, -h / 2.0f);
    glEnd();
    glPopMatrix();
}
