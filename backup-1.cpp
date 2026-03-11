//
// program: game.cpp
// modified by: Bryan Rodriguez
// purpose: CMPS 4490 top-down survival arena starting point
// step 2: player movement + mouse aiming
//

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"

// defined types
typedef float Flt;
typedef float Vec[3];

// constants
#define PI 3.141592653589793

//-----------------------------------------------------------------------------
// Setup timers
const double physicsRate = 1.0 / 60.0;
extern struct timespec timeStart, timeCurrent;
extern struct timespec timePause;
extern double physicsCountdown;
extern double timeSpan;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
//-----------------------------------------------------------------------------

class Global {
public:
    int xres, yres;
    char keys[65536];
    int mouse_x;
    int mouse_y;
    int mouse_cursor_on;

    Global() {
        xres = 800;
        yres = 600;
        memset(keys, 0, 65536);
        mouse_x = xres / 2;
        mouse_y = yres / 2;
        mouse_cursor_on = 1;
    }
} gl;

class Player {
public:
    Vec pos;
    float w, h;
    float speed;
    float angle;
    float color[3];

    Player() {
        pos[0] = 400.0f;
        pos[1] = 300.0f;
        pos[2] = 0.0f;
        w = 40.0f;
        h = 40.0f;
        speed = 4.0f;
        angle = 0.0f;
        color[0] = 0.2f;
        color[1] = 0.8f;
        color[2] = 0.3f;
    }
};

class Game {
public:
    Player player;

    Game() { }
} g;

// X Windows variables
class X11_wrapper {
private:
    Display *dpy;
    Window win;
    GLXContext glc;
public:
    X11_wrapper() { }

    X11_wrapper(int w, int h) {
        GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
        XSetWindowAttributes swa;
        setup_screen_res(gl.xres, gl.yres);
        dpy = XOpenDisplay(NULL);
        if (dpy == NULL) {
            std::cout << "\n\tcannot connect to X server" << std::endl;
            exit(EXIT_FAILURE);
        }

        Window root = DefaultRootWindow(dpy);
        XWindowAttributes getWinAttr;
        XGetWindowAttributes(dpy, root, &getWinAttr);

        int fullscreen = 0;
        gl.xres = w;
        gl.yres = h;

        if (!w && !h) {
            gl.xres = getWinAttr.width;
            gl.yres = getWinAttr.height;
            XGrabKeyboard(dpy, root, False,
                GrabModeAsync, GrabModeAsync, CurrentTime);
            fullscreen = 1;
        }

        XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
        if (vi == NULL) {
            std::cout << "\n\tno appropriate visual found\n" << std::endl;
            exit(EXIT_FAILURE);
        }

        Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
            PointerMotionMask | MotionNotify | ButtonPress | ButtonRelease |
            StructureNotifyMask | SubstructureNotifyMask;

        unsigned int winops = CWBorderPixel | CWColormap | CWEventMask;
        if (fullscreen) {
            winops |= CWOverrideRedirect;
            swa.override_redirect = True;
        }

        win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
            vi->depth, InputOutput, vi->visual, winops, &swa);

        set_title();
        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        glXMakeCurrent(dpy, win, glc);
        show_mouse_cursor(gl.mouse_cursor_on);
    }

    ~X11_wrapper() {
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
    }

    void set_title() {
        XMapWindow(dpy, win);
        XStoreName(dpy, win, "CMPS 4490 - Game Project");
    }

    void check_resize(XEvent *e) {
        if (e->type != ConfigureNotify)
            return;

        XConfigureEvent xce = e->xconfigure;
        if (xce.width != gl.xres || xce.height != gl.yres) {
            reshape_window(xce.width, xce.height);
        }
    }

    void reshape_window(int width, int height) {
        setup_screen_res(width, height);
        glViewport(0, 0, (GLint)width, (GLint)height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
        set_title();
    }

    void setup_screen_res(const int w, const int h) {
        gl.xres = w;
        gl.yres = h;
    }

    void swapBuffers() {
        glXSwapBuffers(dpy, win);
    }

    bool getXPending() {
        return XPending(dpy);
    }

    XEvent getXNextEvent() {
        XEvent e;
        XNextEvent(dpy, &e);
        return e;
    }

    void show_mouse_cursor(const int onoff) {
        if (onoff) {
            XUndefineCursor(dpy, win);
            return;
        }

        Pixmap blank;
        XColor dummy;
        char data[1] = {0};
        Cursor cursor;

        blank = XCreateBitmapFromData(dpy, win, data, 1, 1);
        if (blank == None)
            std::cout << "error: out of memory." << std::endl;

        cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
        XFreePixmap(dpy, blank);
        XDefineCursor(dpy, win, cursor);
    }
} x11(gl.xres, gl.yres);

// function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

//==========================================================================
// MAIN
//==========================================================================
int main()
{
    logOpen();
    init_opengl();
    srand(time(NULL));
    clock_gettime(CLOCK_REALTIME, &timePause);
    clock_gettime(CLOCK_REALTIME, &timeStart);

    int done = 0;
    while (!done) {
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
        }

        clock_gettime(CLOCK_REALTIME, &timeCurrent);
        timeSpan = timeDiff(&timeStart, &timeCurrent);
        timeCopy(&timeStart, &timeCurrent);
        physicsCountdown += timeSpan;

        while (physicsCountdown >= physicsRate) {
            physics();
            physicsCountdown -= physicsRate;
        }

        render();
        x11.swapBuffers();
    }

    cleanup_fonts();
    logClose();
    return 0;
}

void init_opengl(void)
{
    glViewport(0, 0, gl.xres, gl.yres);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0, gl.xres, 0, gl.yres, -1, 1);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void check_mouse(XEvent *e)
{
    if (e->type == MotionNotify) {
        gl.mouse_x = e->xmotion.x;
        gl.mouse_y = gl.yres - e->xmotion.y;
    }
}

int check_keys(XEvent *e)
{
    if (e->type != KeyRelease && e->type != KeyPress)
        return 0;

    int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);

    if (e->type == KeyRelease) {
        gl.keys[key] = 0;
        return 0;
    }

    if (e->type == KeyPress) {
        gl.keys[key] = 1;
    }

    switch (key) {
        case XK_Escape:
            return 1;
    }

    return 0;
}

void physics()
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

    // normalize diagonal movement so it is not faster
    if (moveX != 0.0f || moveY != 0.0f) {
        float len = sqrt(moveX * moveX + moveY * moveY);
        moveX /= len;
        moveY /= len;
    }

    g.player.pos[0] += moveX * g.player.speed;
    g.player.pos[1] += moveY * g.player.speed;

    // keep player on screen
    float halfW = g.player.w * 0.5f;
    float halfH = g.player.h * 0.5f;

    if (g.player.pos[0] < halfW)
        g.player.pos[0] = halfW;
    if (g.player.pos[0] > gl.xres - halfW)
        g.player.pos[0] = gl.xres - halfW;
    if (g.player.pos[1] < halfH)
        g.player.pos[1] = halfH;
    if (g.player.pos[1] > gl.yres - halfH)
        g.player.pos[1] = gl.yres - halfH;

    // aim player toward mouse
    float dx = (float)gl.mouse_x - g.player.pos[0];
    float dy = (float)gl.mouse_y - g.player.pos[1];
    g.player.angle = atan2(dy, dx);
}

void render()
{
    Rect r;

    glClear(GL_COLOR_BUFFER_BIT);

    r.bot = gl.yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint(&r, 16, 16, 0x00ffff00, "CMPS 4490 - Player Movement + Mouse Aim");
    ggprint(&r, 16, 16, 0x00ffffff, "WASD to move");
    ggprint(&r, 16, 16, 0x00ffffff, "Move mouse to aim");
    ggprint(&r, 16, 16, 0x00ffffff, "ESC to quit");
    ggprint(&r, 16, 16, 0x00ffffff, "Player position: (%.0f, %.0f)",
        g.player.pos[0], g.player.pos[1]);
    ggprint(&r, 16, 16, 0x00ffffff, "Mouse position: (%d, %d)",
        gl.mouse_x, gl.mouse_y);

    // draw player square
    glColor3fv(g.player.color);
    glPushMatrix();
    glTranslatef(g.player.pos[0], g.player.pos[1], g.player.pos[2]);

    glBegin(GL_QUADS);
        glVertex2f(-g.player.w / 2.0f, -g.player.h / 2.0f);
        glVertex2f(-g.player.w / 2.0f,  g.player.h / 2.0f);
        glVertex2f( g.player.w / 2.0f,  g.player.h / 2.0f);
        glVertex2f( g.player.w / 2.0f, -g.player.h / 2.0f);
    glEnd();

    // draw player center point
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
        glVertex2f(0.0f, 0.0f);
    glEnd();

    // draw aim line
    float aimLength = 35.0f;
    float ax = cos(g.player.angle) * aimLength;
    float ay = sin(g.player.angle) * aimLength;

    glLineWidth(3.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(ax, ay);
    glEnd();

    glPopMatrix();

    // draw mouse marker
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex2f(gl.mouse_x - 8, gl.mouse_y);
        glVertex2f(gl.mouse_x + 8, gl.mouse_y);
        glVertex2f(gl.mouse_x, gl.mouse_y - 8);
        glVertex2f(gl.mouse_x, gl.mouse_y + 8);
    glEnd();
}
