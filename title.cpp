#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <fstream>

// Utlized so that font files can be used
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

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
    glDisable(GL_TEXTURE_2D);
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

        float r, g, b;
        if (hovered) { r = 1.0f; g = 0.0f; b = 0.0f; }
        else if (isPlay) { r = 204/255.f; g = 0.0f; b = 0.0f; }
        else { r = 136/255.f; g = 136/255.f; b = 136/255.f; }

        float tScale = 0.8f;
        float tWidth = getTextWidth(text, tScale);
        drawText(text, x + width/2.0f - tWidth/2.0f, y + 35.0f, r, g, b, tScale);
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    const int WIDTH = 800;
    const int HEIGHT = 600;

    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) return -1;

    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XVisualInfo* vi = glXChooseVisual(dpy, DefaultScreen(dpy), att);
    Colormap cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);

    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | PointerMotionMask | ButtonPressMask | StructureNotifyMask;
    Window win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, WIDTH, HEIGHT, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

    XStoreName(dpy, win, "Zombie Game - Last Stand");
    XMapWindow(dpy, win);
    Atom wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);

    GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    // Load the external TTF Font
    initFont("font.ttf", 32.0f);

    std::vector<Button> buttons;
    buttons.push_back(Button(WIDTH / 2.0f, 330.0f, "> START GAME", "The horde is coming... Prepare yourself!", true));
    buttons.push_back(Button(WIDTH / 2.0f, 395.0f, "OPTIONS", "Loading..."));
    buttons.push_back(Button(WIDTH / 2.0f, 460.0f, "QUIT", "You can never quit."));

    std::vector<Particle> particles;
    for (int i = 0; i < 15; ++i) particles.push_back(Particle(std::rand() % WIDTH, std::rand() % HEIGHT));

    std::vector<BloodDrip> drips;
    drips.push_back(BloodDrip(WIDTH * 0.1f, 80.0f, 150.0f, 0.0f));
    drips.push_back(BloodDrip(WIDTH * 0.25f, 120.0f, 180.0f, 300.0f));
    drips.push_back(BloodDrip(WIDTH * 0.55f, 60.0f, 130.0f, 600.0f));
    drips.push_back(BloodDrip(WIDTH * 0.75f, 100.0f, 200.0f, 150.0f));
    drips.push_back(BloodDrip(WIDTH * 0.9f, 90.0f, 160.0f, 450.0f));

    float mouseX = 0, mouseY = 0;
    bool running = true;
    float startFlashTimer = 0.0f;
    float totalTime = 0.0f;

    auto prevTime = std::chrono::high_resolution_clock::now();

    while (running) {
        while (XPending(dpy)) {
            XEvent xev;
            XNextEvent(dpy, &xev);

            if (xev.type == MotionNotify) { mouseX = xev.xmotion.x; mouseY = xev.xmotion.y; }
            else if (xev.type == ButtonPress && xev.xbutton.button == 1) {
                for (auto& btn : buttons) {
                    if (btn.isHovered(mouseX, mouseY)) {
                        if (btn.text == "> START GAME") startFlashTimer = 0.5f;
                        std::cout << "[SYSTEM]: " << btn.message << std::endl;
                    }
                }
            }
            else if (xev.type == ClientMessage && (Atom)xev.xclient.data.l[0] == wmDeleteMessage) {
                running = false;
            }
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currentTime - prevTime).count();
        prevTime = currentTime;
        totalTime += dt;

        for (auto& p : particles) p.update(dt, HEIGHT);
        for (auto& d : drips) d.update(dt, HEIGHT);

        bool isLightning = (std::rand() % 1000) > 985;
        float bgR = isLightning ? 26/255.f : 10/255.f;
        float bgG = isLightning ? 26/255.f : 10/255.f;
        float bgB = isLightning ? 42/255.f : 10/255.f;

        if (startFlashTimer > 0) {
            bgR = 1.0f; bgG = 0.0f; bgB = 0.0f;
            startFlashTimer -= dt;
        }

        glClearColor(bgR, bgG, bgB, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto& d : drips) d.draw();
        for (auto& p : particles) p.draw();

        float pulse = 1.0f + 0.05f * std::sin(totalTime * 3.0f);
        float titleScale = 3.0f * pulse;

        std::string title1 = "Zombie";
        std::string title2 = "Game";
        drawText(title1, WIDTH/2.0f - getTextWidth(title1, titleScale)/2.0f, 100.0f, 1.0f, 0.0f, 0.0f, titleScale);
        drawText(title2, WIDTH/2.0f - getTextWidth(title2, titleScale)/2.0f, 100.0f + 30.0f*titleScale, 1.0f, 0.0f, 0.0f, titleScale);

        float tagScale = 0.8f;
        std::string tagline = "- LAST STAND -";
        drawText(tagline, WIDTH/2.0f - getTextWidth(tagline, tagScale)/2.0f, 260.0f, 74/255.f, 74/255.f, 74/255.f, tagScale);

        if (static_cast<int>(totalTime * 2) % 2 == 0) {
            float blinkScale = 0.5f;
            std::string blinkText = "PRESS START TO SURVIVE";
            drawText(blinkText, WIDTH/2.0f - getTextWidth(blinkText, blinkScale)/2.0f, 550.0f, 85/255.f, 85/255.f, 85/255.f, blinkScale);
        }

        for (auto& btn : buttons) btn.draw(mouseX, mouseY);

        glXSwapBuffers(dpy, win);

        auto frameEndTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float>(frameEndTime - currentTime).count();
        if (frameTime < 1.0f / 60.0f) std::this_thread::sleep_for(std::chrono::duration<float>((1.0f / 60.0f) - frameTime));
    }

    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}
