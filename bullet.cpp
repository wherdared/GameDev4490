#include <cmath>
#include <cstring>
#include <GL/glx.h>
#include "bullet.h"
#include "globals.h"

// timer functions from timers.cpp
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);

Bullet::Bullet()
{
    pos[0] = pos[1] = pos[2] = 0.0f;
    vel[0] = vel[1] = vel[2] = 0.0f;
    active = false;
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
}

BulletManager::BulletManager()
{
    nbullets = 0;
    // later we can turn to false during downtime between waves
    shootingEnabled = true;
    fireDelay = 0.25; // shoots every 0.25 seconds
    clock_gettime(CLOCK_REALTIME, &bulletTimer);
}

void BulletManager::spawn(const Player &player)
{
    if (nbullets >= MAX_BULLETS)
        return;

    Bullet *b = &bullets[nbullets];

    float xdir = cos(player.angle);
    float ydir = sin(player.angle);

    b->pos[0] = player.pos[0] + xdir * 25.0f;
    b->pos[1] = player.pos[1] + ydir * 25.0f;
    b->pos[2] = 0.0f;

    float bulletSpeed = 8.0f;
    b->vel[0] = xdir * bulletSpeed;
    b->vel[1] = ydir * bulletSpeed;
    b->vel[2] = 0.0f;

    b->active = true;
    b->color[0] = 1.0f;
    b->color[1] = 1.0f;
    b->color[2] = 1.0f;

    nbullets++;
}

void BulletManager::update(const Player &player)
{
    if (shootingEnabled) {
        struct timespec bt;
        clock_gettime(CLOCK_REALTIME, &bt);
        double ts = timeDiff(&bulletTimer, &bt);

        if (ts > fireDelay) {
            timeCopy(&bulletTimer, &bt);
            spawn(player);
        }
    }

    int i = 0;
    while (i < nbullets) {
        Bullet *b = &bullets[i];

        b->pos[0] += b->vel[0];
        b->pos[1] += b->vel[1];

        // Remove bullet if it leaves screen
        if (b->pos[0] < 0.0f || b->pos[0] > gl.xres ||
            b->pos[1] < 0.0f || b->pos[1] > gl.yres) {
            bullets[i] = bullets[nbullets - 1];
            nbullets--;
            continue;
        }

        i++;
    }
}

void BulletManager::render()
{
    for (int i = 0; i < nbullets; i++) {
        Bullet *b = &bullets[i];

        glColor3fv(b->color);
        glBegin(GL_POINTS);
            glVertex2f(b->pos[0],      b->pos[1]);
            glVertex2f(b->pos[0] - 1.0f, b->pos[1]);
            glVertex2f(b->pos[0] + 1.0f, b->pos[1]);
            glVertex2f(b->pos[0],      b->pos[1] - 1.0f);
            glVertex2f(b->pos[0],      b->pos[1] + 1.0f);
        glEnd();
    }
}
