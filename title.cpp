#include <GL/gl.h>
#include <GL/glx.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>

// utlized to help with fonts
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "globals.h"
#include "title.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

stbtt_bakedchar cdata[96];
GLuint fontTexture;

void initFont(const char* filename, float size) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        std::cerr << "Error: Could not find '" << filename << "' in the folder!\n";
        exit(-1);
    }
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    unsigned char* ttf_buffer = new unsigned char[fileSize];
    fread(ttf_buffer, 1, fileSize, fp);
    fclose(fp);

    unsigned char temp_bitmap[512 * 512];
    stbtt_BakeFontBitmap(ttf_buffer, 0, size, temp_bitmap, 512, 512, 32, 96, cdata);
    delete[] ttf_buffer;

    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

float getTextWidth(const std::string& text, float scale) {
    float x = 0, y = 0;
    for(char c : text) {
        if (c >= 32 && c < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512, 512, c-32, &x, &y, &q, 1);
        }
    }
    return x * scale;
}

void drawText(const std::string& text, float x, float y, float r, float g, float b, float scale = 1.0f) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glColor4f(r, g, b, 1.0f);

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);

    float curX = 0, curY = 0;
    glBegin(GL_QUADS);
    for(char c : text) {
        if (c >= 32 && c < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512, 512, c-32, &curX, &curY, &q, 1);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
        }
    }
    glEnd();
    glPopMatrix();
}

struct Particle {
    float x, y, speed;
    Particle(float startX, float startY) : x(startX), y(startY) {
        speed = 20.0f + (std::rand() % 30);
    }
    void update(float dt, float windowHeight) {
        x += 10.0f * dt;
        y -= speed * dt;
        if (y < 0) { x -= 30.0f; y = windowHeight + 10.0f; }
    }
    void draw() {
        glColor4f(200/255.f, 200/255.f, 200/255.f, 76/255.f);
        glBegin(GL_POLYGON);
        for(int i = 0; i < 12; ++i) {
            float theta = i * 2.0f * M_PI / 12.0f;
            glVertex2f(x + 2.0f * std::cos(theta), y + 2.0f * std::sin(theta));
        }
        glEnd();
    }
};

struct BloodDrip {
    float x, y, speed, startY, height;
    BloodDrip(float startX, float h, float spd, float delay) {
        height = h; x = startX; y = -height - delay;
        speed = spd; startY = -height;
    }
    void update(float dt, float windowHeight) {
        y += speed * dt;
        if (y > windowHeight) y = startY;
    }
    void draw() {
        glColor4f(139/255.f, 0, 0, 200/255.f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + 4.0f, y);
        glVertex2f(x + 4.0f, y + height); glVertex2f(x, y + height);
        glEnd();
    }
};

struct Button {
    float x, y, width, height;
    std::string text, message;
    bool isPlay;

    Button(float cx, float cy, std::string str, std::string msg, bool play = false) {
        width = 300.0f; height = 50.0f;
        x = cx - width / 2.0f; y = cy;
        text = str; message = msg; isPlay = play;
    }
    bool isHovered(float mx, float my) {
        return (mx >= x && mx <= x + width && my >= y && my <= y + height);
    }
    void draw(float mx, float my) {
        bool hovered = isHovered(mx, my);
        if (hovered) glColor4f(139/255.f, 0, 0, 1.0f);
        else glColor4f(51/255.f, 51/255.f, 51/255.f, 1.0f);

        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y); glVertex2f(x + width, y);
        glVertex2f(x + width, y + height); glVertex2f(x, y + height);
        glEnd();
        glLineWidth(1.0f);

        float r, g, b;
        if (hovered) { r = 1.0f; g = 0.0f; b = 0.0f; }
        else if (isPlay) { r = 204/255.f; g = 0.0f; b = 0.0f; }
        else { r = 136/255.f; g = 136/255.f; b = 136/255.f; }

        float tScale = 0.8f;
        float tWidth = getTextWidth(text, tScale);
        drawText(text, x + width/2.0f - tWidth/2.0f, y + 35.0f, r, g, b, tScale);
    }
};

std::vector<Button> buttons;
std::vector<Particle> particles;
std::vector<BloodDrip> drips;

float totalTime = 0.0f;
float startFlashTimer = 0.0f;

void initTitle() {
    initFont("font.ttf", 32.0f);
    float cx = gl.xres / 2.0f;
    float cy = gl.yres / 2.0f;

    buttons.push_back(Button(cx, cy + 30.0f, "> START GAME", "Prepare yourself!", true));
    buttons.push_back(Button(cx, cy + 95.0f, "OPTIONS", "Loading..."));
    buttons.push_back(Button(cx, cy + 160.0f, "QUIT", "You can never quit."));

    for (int i = 0; i < 15; ++i) particles.push_back(Particle(std::rand() % gl.xres, std::rand() % gl.yres));
    drips.push_back(BloodDrip(gl.xres * 0.1f, 80.0f, 150.0f, 0.0f));
    drips.push_back(BloodDrip(gl.xres * 0.25f, 120.0f, 180.0f, 300.0f));
    drips.push_back(BloodDrip(gl.xres * 0.55f, 60.0f, 130.0f, 600.0f));
    drips.push_back(BloodDrip(gl.xres * 0.75f, 100.0f, 200.0f, 150.0f));
    drips.push_back(BloodDrip(gl.xres * 0.9f, 90.0f, 160.0f, 450.0f));
}

void updateTitle(float dt) {
    totalTime += dt;
    for (auto& p : particles) p.update(dt, gl.yres);
    for (auto& d : drips) d.update(dt, gl.yres);

    if (startFlashTimer > 0) {
        startFlashTimer -= dt;
        if (startFlashTimer <= 0) {
            gl.state = STATE_GAME; 
        }
    }
}

void renderTitle() {
    // Switch to Y-Down projection for title rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, gl.xres, gl.yres, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bool isLightning = (std::rand() % 1000) > 985;
    float bgR = isLightning ? 26/255.f : 10/255.f;
    float bgG = isLightning ? 26/255.f : 10/255.f;
    float bgB = isLightning ? 42/255.f : 10/255.f;

    if (startFlashTimer > 0) { bgR = 1.0f; bgG = 0.0f; bgB = 0.0f; }

    glClearColor(bgR, bgG, bgB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto& d : drips) d.draw();
    for (auto& p : particles) p.draw();

    float pulse = 1.0f + 0.05f * std::sin(totalTime * 3.0f);
    float titleScale = 3.0f * pulse;

    drawText("Zombie", gl.xres/2.0f - getTextWidth("Zombie", titleScale)/2.0f, 100.0f, 1.0f, 0.0f, 0.0f, titleScale);
    drawText("Game", gl.xres/2.0f - getTextWidth("Game", titleScale)/2.0f, 100.0f + 30.0f*titleScale, 1.0f, 0.0f, 0.0f, titleScale);
    
    float tagScale = 0.8f;
    drawText("- LAST STAND -", gl.xres/2.0f - getTextWidth("- LAST STAND -", tagScale)/2.0f, 260.0f, 74/255.f, 74/255.f, 74/255.f, tagScale);

    if (static_cast<int>(totalTime * 2) % 2 == 0) {
        float blinkScale = 0.5f;
        drawText("PRESS START TO SURVIVE", gl.xres/2.0f - getTextWidth("PRESS START TO SURVIVE", blinkScale)/2.0f, gl.yres - 50.0f, 85/255.f, 85/255.f, 85/255.f, blinkScale);
    }

    for (auto& btn : buttons) btn.draw(gl.mouse_x, gl.mouse_y_down);

    glDisable(GL_BLEND);
    
    // Restore projection for game.cpp 
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void checkTitleClick(int mx, int my) {
    for (auto& btn : buttons) {
        if (btn.isHovered(mx, my)) {
            if (btn.text == "> START GAME") {
                startFlashTimer = 0.5f;
            } else if (btn.text == "QUIT") {
                gl.done = true; // Tell the game loop to quit
            }
        }
    }
}
