#include <cmath>
#include <GL/glx.h>
#include "player.h"
#include "globals.h"
#include "fonts.h"
#include "sprite.h"

extern Sprite playerIdle;
extern Sprite playerMove;
extern Sprite playerShoot;
extern Sprite *currentPlayerSprite;
extern bool spritesLoaded;

#ifndef PI
#define PI 3.14159265358979323846
#endif

Player::Player()
{
    pos[0] =  gl.xres/2;      //400.0f;
    pos[1] =  gl.yres/2;      //300.0f;
    pos[2] = 0.0f;
    w = 40.0f;
    h = 40.0f;
    speed = 4.0f;
    angle = 0.0f;
    maxHealth = 100.0f;
    health = maxHealth;
    wasDamaged = false;
    damageCooldown = 0;
    flashTimer = 0;
    color[0] = 0.2f;
    color[1] = 0.8f;
    color[2] = 0.3f;
}

void Player::update()
{
    float moveX = 0.0f;
    float moveY = 0.0f;

    if (gl.keys[XK_w] || gl.keys[XK_W])
        moveY += 1.0f;
    if (gl.keys[XK_s] || gl.keys[XK_S])
        moveY -= 1.0f;
    if (gl.keys[XK_a] || gl.keys[XK_A])
        moveX -= 1.0f;
    if (gl.keys[XK_d] || gl.keys[XK_D])
        moveX += 1.0f;

    // Prevent diagonal movement from being faster
    if (moveX != 0.0f || moveY != 0.0f) {
        float len = sqrt(moveX * moveX + moveY * moveY);
        moveX /= len;
        moveY /= len;
    }

    pos[0] += moveX * speed;
    pos[1] += moveY * speed;
    
    float halfW = w * 0.5f;
    float halfH = h * 0.5f;

    // Keep player on screen
    if (pos[0] < halfW)
        pos[0] = halfW;
    if (pos[0] > gl.xres - halfW)
        pos[0] = gl.xres - halfW;
    if (pos[1] < halfH)
        pos[1] = halfH;
    if (pos[1] > gl.yres - halfH)
        pos[1] = gl.yres - halfH;

    // Face the mouse
    float dx = (float)gl.mouse_x - pos[0];
    float dy = (float)gl.mouse_y - pos[1];
    angle = atan2(dy, dx);
    
    if (damageCooldown > 0)
        damageCooldown--;

    if (flashTimer > 0)
        flashTimer--;
}

void Player::takeDamage(float amount)
{
    if (damageCooldown > 0)
        return;

    health -= amount;

    if (health < 0.0f)
        health = 0.0f;

    damageCooldown = 30; // prevents damage every single frame
    flashTimer = 10;     // player flashes red briefly
}

void Player::renderHealthBar()
{

    float barW = 200.0f;
    float barH = 20.0f;
    float x   = 20.0f;
    float y   = 20.0f;

    Rect r;
 
    // position for HP text
    r.left = x;
    r.bot  = y + 25;
    r.center = 0;
 
    ggprint(&r, 16, 0, 0x00ffffff, "H P");
    r.left += 1;
    ggprint(&r, 16, 0, 0x00ffffff, "H P");
    r.left -= 1;
 
    // position for HP fraction
    r.left = x + 50;
    r.bot  = y + 25;
    r.center = 0;

    // health numbers (green)
    ggprint(&r, 16, 0, 0x00ffffff, "%i/%i", (int)health, (int)maxHealth);

    float percent = health / maxHealth;

    if (percent < 0.0f)
        percent = 0.0f;
    if (percent > 1.0f)
        percent = 1.0f;

    // Background bar
    glColor3f(0.6f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + barW, y);
        glVertex2f(x + barW, y + barH);
        glVertex2f(x, y + barH);
    glEnd();

    // Health amount RGB
    glColor3f(0.0f, 0.85f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + barW * percent, y);
        glVertex2f(x + barW * percent, y + barH);
        glVertex2f(x, y + barH);
    glEnd();

    // Border
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + barW, y);
        glVertex2f(x + barW, y + barH);
        glVertex2f(x, y + barH);
    glEnd();
}

void Player::render()
{
    if (spritesLoaded && currentPlayerSprite) {
        float angleDegrees = angle * 180.0f / (float)PI;

        glPushMatrix();
        glTranslatef(pos[0], pos[1], 0.0f);
        glRotatef(angleDegrees, 0.0f, 0.0f, 1.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);

        if (flashTimer > 0)
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);  // red flash
        else
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // normal sprite

        glBindTexture(GL_TEXTURE_2D,
            currentPlayerSprite->tex[currentPlayerSprite->currentFrame]);

        float sw = currentPlayerSprite->frameWidth / 2.0f;
        float sh = currentPlayerSprite->frameHeight / 2.0f;

        glBegin(GL_QUADS);
            glTexCoord2f(0, 1); glVertex2f(-sw, -sh);
            glTexCoord2f(0, 0); glVertex2f(-sw,  sh);
            glTexCoord2f(1, 0); glVertex2f( sw,  sh);
            glTexCoord2f(1, 1); glVertex2f( sw, -sh);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // reset color

        glPopMatrix();
    }
}


