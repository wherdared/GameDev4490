#ifndef ROUNDS_H
#define ROUNDS_H

#include <ctime>

const int MAX_ZOMBIES = 25;     // max that can be in screen not per round

class RoundManager {
    public:
        // tweakable settings
        int startingZombies;
        int zombiesPerRound;    // added each round
        int maxOnScreen;
        int roundCap;           
        int winRound;           // beat the game after this round
        float breakDuration;    // seconds between rounds

        // round state
        int currentRound;
        int totalZombiesThisRound;
        int zombiesSpawned;
        int zombiesKilled;
        bool inBreak;
        float breakTimer;
        struct timespec spawnTimer;
        
        RoundManager();
        void init();
        void update();
};

extern RoundManager roundManager;

#endif
