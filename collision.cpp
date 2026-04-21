#include <cmath>
#include <cstdio>
#include "collision.h"
#include "globals.h"

extern Player player;
extern BulletManager bulletManager;
extern Zombie zombie[];
extern int nzombies;

void checkCollisions() {
    // get edges of player and zombie
    float pLeft     = player.pos[0] - player.w / 2.0f;
    float pRight    = player.pos[0] + player.w / 2.0f;
    float pBottom   = player.pos[1] - player.h / 2.0f;
    float pTop      = player.pos[1] + player.h / 2.0f;

    for (int j=0; j<nzombies; j++) {
        if (!zombie[j].alive) continue;     // if zombie is dead js skip this

        float zLeft     = zombie[j].pos[0] - zombie[j].w / 2.0f;
        float zRight    = zombie[j].pos[0] + zombie[j].w / 2.0f;
        float zBottom   = zombie[j].pos[1] - zombie[j].h / 2.0f;
        float zTop      = zombie[j].pos[1] + zombie[j].h / 2.0f;
    
        // -- player and zombie collision
        if (!(pRight < zLeft || pLeft > zRight || pTop < zBottom || pBottom > zTop)) {
    
            // use center distance to determine which axis to resolve on
            float dx = player.pos[0] - zombie[j].pos[0];
            float dy = player.pos[1] - zombie[j].pos[1];
            float pushSpeed = 0.85f;         
    
            if (fabs(dx) > fabs(dy)) {
                // horizontal collision - stop zombie on x axis
                if (dx > 0) {
                    zombie[j].pos[0] = pLeft - zombie[j].w / 2.0f;
                    player.pos[0] = zRight + player.w / 2.0f;
                    player.pos[0] += pushSpeed;
                } else {
                    zombie[j].pos[0] = pRight + zombie[j].w / 2.0f;
                    player.pos[0] = zLeft - player.w / 2.0f;
                    player.pos[0] -= pushSpeed;
            }
            } else {
                // vertical collision - stop zombie on y axis
                if (dy > 0) {
                    zombie[j].pos[1] = pBottom - zombie[j].h / 2.0f;
                    player.pos[1] = zTop + player.h / 2.0f;
                    player.pos[1] += pushSpeed;
                } else {
                    zombie[j].pos[1] = pTop + zombie[j].h / 2.0f;
                    player.pos[1] = zBottom - player.h / 2.0f;
                    player.pos[1] -= pushSpeed;
                }         
            }
    
            // clamp player to screen and push zombie back if player hits wall
            float halfW = player.w / 2.0f;
            float halfH = player.h / 2.0f;
            float pushExtra = 3.0f;

            if (player.pos[0] < halfW) {
                player.pos[0] = halfW;
                zombie[j].pos[0] = halfW + player.w / 2.0f + zombie[j].w / 2.0f + pushExtra;
            }
            if (player.pos[0] > gl.xres - halfW) {
                player.pos[0] = gl.xres - halfW;
                zombie[j].pos[0] = gl.xres - halfW - player.w / 2.0f - zombie[j].w / 2.0f - pushExtra;
            }
            if (player.pos[1] < halfH) {
                player.pos[1] = halfH;
                zombie[j].pos[1] = halfH + player.h / 2.0f + zombie[j].h / 2.0f + pushExtra;
            }   
            if (player.pos[1] > gl.yres - halfH) {
                player.pos[1] = gl.yres - halfH;
                zombie[j].pos[1] = gl.yres - halfH - player.h / 2.0f - zombie[j].h / 2.0f - pushExtra;
            }
        }

        // bullet - zombie collision
        int i = 0;
        while(i < bulletManager.nbullets) {
            Bullet *b = &bulletManager.bullets[i];

            // check if bullet is iniside zombie bounds
            if (b->pos[0] > zLeft && b->pos[0] < zRight &&
                b->pos[1] > zBottom && b->pos[1] < zTop) {

                // bullet hit zombie
                // for now each bullet will deal 34 damage but later we can change this 
                // if we decide to add more weapons, so each weapon 
                // will deal different damage
                zombie[j].health -= 34.0f;
                //printf("zombie[%d] health: %1f\n", j, zombie[j].health);    

                // delete bullet
                bulletManager.bullets[i] = bulletManager.bullets[bulletManager.nbullets - 1];
                bulletManager.nbullets--;

                // check if zombie is dead
                if (zombie[j].health <= 0.0f) {
                    //printf("zombie[%d] is dead!\n", j);
                    zombie[j].health = 0.0f;
                    zombie[j].alive = false;        // mark zombie as dead
                }

                continue;
            }
            i++;
        }
    }
}

