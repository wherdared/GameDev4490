#include <cstring>
#include <cstdio>
#include <cmath>
#include "sprite.h"

extern int loadTexturePNG_UsingImageMagick(const char*, GLuint&);

Sprite::Sprite()
{
    nframes = 0;
    currentFrame = 0;
    frameTimer = 0.0f;
    frameDelay = 0.1f;
    frameWidth = 64;
    frameHeight = 64;
}

void Sprite::load(const char *folder, int frameCount)
{
    nframes = frameCount;

    char filename[256];

    for (int i = 0; i < frameCount; i++) {
        sprintf(filename, "%s/%d.png", folder, i);
        loadTexturePNG_UsingImageMagick(filename, tex[i]);
    }
}

void Sprite::update(float dt)
{
    frameTimer += dt;

    if (frameTimer > frameDelay) {
        frameTimer = 0.0f;
        currentFrame++;
        if (currentFrame >= nframes)
            currentFrame = 0;
    }
}

void Sprite::render(float x, float y, float angle)
{
    glPushMatrix();

    glTranslatef(x, y, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[currentFrame]);

    float w = frameWidth / 2.0f;
    float h = frameHeight / 2.0f;

    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex2f(-w,-h);
        glTexCoord2f(0,0); glVertex2f(-w, h);
        glTexCoord2f(1,0); glVertex2f( w, h);
        glTexCoord2f(1,1); glVertex2f( w,-h);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}
