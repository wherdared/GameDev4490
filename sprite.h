#ifndef SPRITE_H
#define SPRITE_H

#include <GL/gl.h>

#define MAX_FRAMES 32

class Sprite {
public:
    GLuint tex[MAX_FRAMES];
    int nframes;
    int currentFrame;

    float frameTimer;
    float frameDelay;

    int frameWidth;
    int frameHeight;

    Sprite();

    void load(const char *folder, int frameCount);
    void update(float dt);
    void render(float x, float y, float angle);
};

#endif
