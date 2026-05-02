#include "powerup.h"
#include <GL/glx.h>
#include <cmath>

PowerUp nukePowerUp;
PowerUp freezePowerUp;

PowerUp::PowerUp() {
    active = false;
    radius = 200.0f; // Radius 
    w = 20.0f;
    h = 20.0f;

    // nuke power up color
    color[0] = 1.0f;
    color[1] = 0.3f;
    color[2] = 0.0f; // orange box

    // freeze power up color
    freezePowerUp.color[0] = 0.0f;
    freezePowerUp.color[1] = 0.8f;
    freezePowerUp.color[2] = 1.0f;

    explosionTimer = 0.0f;
    maxExplosionTime = 0.4f;
    pulse = 0.0f;
}

void PowerUp::spawn(float x, float y) {
    pos[0] = x;
    pos[1] = y;
    pos[2] = 0.0f;
    active = true;
    explosionTimer = 0.0f;
}

void PowerUp::update() {
    if (active) {
        pulse += 0.15f; // Animate 
    }
    
    // Process explosion expanding animation
    if (explosionTimer > 0.0f) {
        explosionTimer -= (1.0f / 60.0f);
        if (explosionTimer < 0.0f) explosionTimer = 0.0f;
    }
}

void PowerUp::render() {
    if (active) {
        glPushMatrix();
        glTranslatef(pos[0], pos[1], pos[2]);
        
        float scale = 1.0f + 0.2f * sin(pulse);
        glScalef(scale, scale, 1.0f);

        glColor3fv(color);
        glBegin(GL_QUADS);
            glVertex2f(-w / 2.0f, -h / 2.0f);
            glVertex2f(-w / 2.0f,  h / 2.0f);
            glVertex2f( w / 2.0f,  h / 2.0f);
            glVertex2f( w / 2.0f, -h / 2.0f);
        glEnd();
        
        // Inner core
        glColor3f(1.0f, 0.9f, 0.2f);
        glBegin(GL_QUADS);
            glVertex2f(-w / 4.0f, -h / 4.0f);
            glVertex2f(-w / 4.0f,  h / 4.0f);
            glVertex2f( w / 4.0f,  h / 4.0f);
            glVertex2f( w / 4.0f, -h / 4.0f);
        glEnd();

        glPopMatrix();
    }

    // wave animation
    if (explosionTimer > 0.0f) {
        float progress = 1.0f - (explosionTimer / maxExplosionTime);
        float currentRadius = radius * progress; // Expands outward
        
        glPushMatrix();
        glTranslatef(pos[0], pos[1], pos[2]);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glColor4f(1.0f, 0.1f, 0.1f, 1.0f - progress); 
        
        glLineWidth(4.0f);
        glBegin(GL_LINE_LOOP);
        for(int i = 0; i < 36; i++) {
            float theta = 2.0f * 3.1415926f * float(i) / 36.0f;
            glVertex2f(currentRadius * cos(theta), currentRadius * sin(theta));
        }
        glEnd();
        glLineWidth(1.0f);
        glDisable(GL_BLEND);
        
        glPopMatrix();
    }
}
