#include <cmath>
#include <GL/glx.h>
#include "player.h"
#include "globals.h"

Player::Player()
{
    pos[0] =  gl.xres/2;      //400.0f;
    pos[1] =  gl.yres/2;      //300.0f;
    pos[2] = 0.0f;
    w = 40.0f;
    h = 40.0f;
    speed = 4.0f;
    angle = 0.0f;
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
    
}

void Player::render()
{
    glColor3fv(color);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);

    // Draw player body
    glBegin(GL_QUADS);
        glVertex2f(-w / 2.0f, -h / 2.0f);
        glVertex2f(-w / 2.0f,  h / 2.0f);
        glVertex2f( w / 2.0f,  h / 2.0f);
        glVertex2f( w / 2.0f, -h / 2.0f);
    glEnd();

    // Draw center point
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
        glVertex2f(0.0f, 0.0f);
    glEnd();

    // Draw aim line
    float aimLength = 35.0f;
    float ax = cos(angle) * aimLength;
    float ay = sin(angle) * aimLength;

    glLineWidth(3.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(ax, ay);
    glEnd();

    glPopMatrix();

    // Draw mouse marker
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex2f(gl.mouse_x - 8, gl.mouse_y);
        glVertex2f(gl.mouse_x + 8, gl.mouse_y);
        glVertex2f(gl.mouse_x, gl.mouse_y - 8);
        glVertex2f(gl.mouse_x, gl.mouse_y + 8);
    glEnd();
}
