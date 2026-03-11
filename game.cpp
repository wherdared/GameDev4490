//
// program: game.cpp
// modified by: Bryan Rodriguez
// purpose: CMPS 4490 top-down survival arena starting point
//

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"
#include "globals.h"
#include "player.h"
#include "bullet.h"

// timers from timers.cpp
const double physicsRate = 1.0 / 60.0;
extern struct timespec timeStart, timeCurrent;
extern struct timespec timePause;
extern double physicsCountdown;
extern double timeSpan;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);

Global gl;
Player player;
BulletManager bulletManager;

// X11 wrapper
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

// prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

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
    player.update();
    bulletManager.update(player);
}

void render()
{
    Rect r;

    glClear(GL_COLOR_BUFFER_BIT);

    r.bot = gl.yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint(&r, 16, 16, 0x00ffff00, "CMPS 4490 - Player Test\n");
    ggprint(&r, 16, 16, 0x00ffffff, "WASD to move\n");
    ggprint(&r, 16, 16, 0x00ffffff, "Move mouse to aim\n");
    ggprint(&r, 16, 16, 0x00ffffff, "Auto shooting enabled\n");
    ggprint(&r, 16, 16, 0x00ffffff, "ESC to quit\n");
    ggprint(&r, 16, 16, 0x00ffffff, "Player position: (%.0f, %.0f)\n",
        player.pos[0], player.pos[1]);
    ggprint(&r, 16, 16, 0x00ffffff, "Mouse position: (%d, %d)\n",
        gl.mouse_x, gl.mouse_y);
    ggprint(&r, 16, 16, 0x00ffffff, "Bullets: %d\n", bulletManager.nbullets);

    player.render();
    bulletManager.render();
}
