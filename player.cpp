#include <cmath>
#include <GL/glx.h>
#include "player.h"
#include "globals.h"

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
    glPushMatrix();
    glTranslatef(pos[0], pos[1], pos[2]);

    // Rotate player so its front faces the mouse
    // Our "front" is the +x direction, so convert radians to degrees
    float angleDegrees = angle * 180.0f / (float)PI;
    glRotatef(angleDegrees, 0.0f, 0.0f, 1.0f);

    // Draw player body
    glColor3fv(color);
    glBegin(GL_QUADS);
        glVertex2f(-w / 2.0f, -h / 2.0f);
        glVertex2f(-w / 2.0f,  h / 2.0f);
        glVertex2f( w / 2.0f,  h / 2.0f);
        glVertex2f( w / 2.0f, -h / 2.0f);
    glEnd();

    // Draw center point
    glPointSize(5.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
        glVertex2f(0.0f, 0.0f);
    glEnd();
    glPointSize(1.0f);

    // Draw small barrel/front marker
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(w / 2.0f - 2.0f, -4.0f);
        glVertex2f(w / 2.0f - 2.0f,  4.0f);
        glVertex2f(w / 2.0f + 12.0f, 4.0f);
        glVertex2f(w / 2.0f + 12.0f, -4.0f);
    glEnd();

    // Draw aim line
    float aimLength = 35.0f;
    glLineWidth(3.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(aimLength, 0.0f);
    glEnd();
    glLineWidth(1.0f);

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

