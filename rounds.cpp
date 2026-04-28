#include <cmath>
#include <ctime>
#include <cstdio>
#include "rounds.h"
#include "globals.h"
#include "zombie.h"

extern double timeDiff(struct timespec *start, struct timespec *end);
extern Zombie zombie[];
extern int nzombies;

RoundManager roundManager;

RoundManager::RoundManager() {
    startingZombies      = 10;
    zombiesPerRound      = 4;           // zombies added per round so round would be 10+4=14
    maxOnScreen          = MAX_ZOMBIES;
    roundCap             = 50;          // after this round no more zombiePerRound
    winRound             = 3;           // player wins when beating this round (original was 70)
    breakDuration        = 3.0f;        // grace period between rounds

    currentRound         = 0;
    totalZombiesThisRound = 0;
    zombiesSpawned       = 0;
    zombiesKilled        = 0;
    inBreak              = false;
    breakTimer           = 0.0f;
    spawnTimer           = {0, 0};
}

void RoundManager::init() {
    currentRound         = 1;
    totalZombiesThisRound = startingZombies;
    zombiesSpawned       = 0;
    zombiesKilled        = 0;
    inBreak              = false;
    nzombies             = 0;
    clock_gettime(CLOCK_REALTIME, &spawnTimer);
}

void RoundManager::update() {
    // check win condition
    if (currentRound > winRound) {
        gl.done = true;
        return;
    }

    // if in break between rounds
    if (inBreak) {
        breakTimer -= (float)(1.0 / 60.0);
        if (breakTimer <= 0.0f) {
            inBreak = false;
            currentRound++;

            // check win condition after incrementing
            if (currentRound > winRound) {
                gl.done = true;
                return;
            }

            // calculate zombies for new round
            if (currentRound <= roundCap) {
                totalZombiesThisRound = startingZombies + (currentRound - 1) * zombiesPerRound;
            } else {
                totalZombiesThisRound = startingZombies + (roundCap - 1) * zombiesPerRound;
            }

            zombiesSpawned = 0;
            zombiesKilled  = 0;
            nzombies       = 0;
            clock_gettime(CLOCK_REALTIME, &spawnTimer);
        }
        return;
    }

    // check if round is complete
    if (zombiesKilled >= totalZombiesThisRound) {
        inBreak    = true;
        breakTimer = breakDuration;
        return;
    }

    // count alive zombies on screen
    int aliveCount = 0;
    for (int i = 0; i < nzombies; i++) {
        if (zombie[i].alive && zombie[i].health > 0.0f)
            aliveCount++;
    }

    // spawn zombies every 1 second up to maxOnScreen and totalZombiesThisRound
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double elapsed = timeDiff(&spawnTimer, &now);

    if (elapsed >= 1.0 && zombiesSpawned < totalZombiesThisRound && aliveCount < maxOnScreen) {
        if (nzombies < maxOnScreen) {
            zombie[nzombies].init();
            nzombies++;
        } else {
            // reuse a dead slot
            for (int i = 0; i < nzombies; i++) {
                if (!zombie[i].alive && zombie[i].health <= 0.0f) {
                    zombie[i].init();
                    break;
                }
            }
        }
        zombiesSpawned++;
        clock_gettime(CLOCK_REALTIME, &spawnTimer);
    }
}
