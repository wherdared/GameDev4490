#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstring>

typedef float Flt;
typedef float Vec[3];

#define PI 3.141592653589793

// 1. ADDED: Game states
enum GameState {
    STATE_TITLE,
    STATE_GAME
};

class Global {
public:
    int xres, yres;
    char keys[65536];
    int mouse_x;
    int mouse_y;
    int mouse_y_down;
    int mouse_cursor_on;
    int nframes, fps;
    
    GameState state;  
    bool done;      

    Global() {
        xres = 1200;
        yres = 1000;
        memset(keys, 0, 65536);
        mouse_x = xres / 2;
        mouse_y = yres / 2;
        mouse_y_down = yres / 2;
        mouse_cursor_on = 1;
        nframes = 1;
        fps = 0;
        state = STATE_TITLE; 
        done = false;
    }
};

extern Global gl;

#endif
