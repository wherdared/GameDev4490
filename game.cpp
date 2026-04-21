//
// program: game.cpp
// purpose: CMPS 4490 top-down survival arena
// modified by: Bryan Rodriguez, Ibran Perez, Ramon Moreno
//

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"
#include "globals.h"
#include "player.h"
#include "zombie.h"
#include "bullet.h"
#include "collision.h"
#include "sprite.h"
#include "title.h"

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
Zombie zombie[MAX_ZOMBIES];
int nzombies = 0;
struct timespec zombieSpawnTimer;

GLuint backgroundTex = 0;

Sprite playerIdle;
Sprite playerMove;
Sprite playerShoot;
Sprite *currentPlayerSprite = NULL;

bool spritesLoaded = false;
double shootAnimTimer = 0.0;
struct timespec lastBulletStamp;

// helper functions for sprite/background loading
bool fileExists(const std::string &path)
{
    struct stat st;
    return (stat(path.c_str(), &st) == 0);
}

bool keysMoving()
{
    return gl.keys[XK_w] || gl.keys[XK_W] ||
           gl.keys[XK_a] || gl.keys[XK_A] ||
           gl.keys[XK_s] || gl.keys[XK_S] ||
           gl.keys[XK_d] || gl.keys[XK_D];
}

bool bulletTimerChanged(const struct timespec &a, const struct timespec &b)
{
    return (a.tv_sec != b.tv_sec || a.tv_nsec != b.tv_nsec);
}

std::string remapSpriteFilename(const std::string &path)
{
    if (fileExists(path))
        return path;

    size_t slash = path.find_last_of('/');
    if (slash == std::string::npos)
        return path;

    std::string folder = path.substr(0, slash);
    std::string name   = path.substr(slash + 1);

    int frameNum = -1;
    if (std::sscanf(name.c_str(), "%d.png", &frameNum) != 1)
        return path;

    std::string alt;
    if (folder.find("idle") != std::string::npos) {
        alt = folder + "/survivor-idle_rifle_" + std::to_string(frameNum) + ".png";
    } else if (folder.find("move") != std::string::npos) {
        alt = folder + "/survivor-move_rifle_" + std::to_string(frameNum) + ".png";
    } else if (folder.find("shoot") != std::string::npos) {
        alt = folder + "/survivor-shoot_rifle_" + std::to_string(frameNum) + ".png";
    } else {
        return path;
    }

    if (fileExists(alt))
        return alt;

    return path;
}

// this is the exact function sprite.cpp is expecting
int loadTexturePNG_UsingImageMagick(const char *filename, GLuint &tex)
{
    std::string finalPath = remapSpriteFilename(filename);
    if (!fileExists(finalPath)) {
        std::cout << "Texture file not found: " << filename << std::endl;
        return 0;
    }

    char cmd[1024];
    int imgW = 0;
    int imgH = 0;

    std::snprintf(cmd, sizeof(cmd),
        "identify -format \"%%w %%h\" \"%s\" 2>/dev/null", finalPath.c_str());
    FILE *fpInfo = popen(cmd, "r");
    if (!fpInfo)
        return 0;

    if (fscanf(fpInfo, "%d %d", &imgW, &imgH) != 2) {
        pclose(fpInfo);
        return 0;
    }
    pclose(fpInfo);

    if (imgW <= 0 || imgH <= 0)
        return 0;

    std::vector<unsigned char> pixels(imgW * imgH * 4);

    std::snprintf(cmd, sizeof(cmd),
        "convert \"%s\" -alpha on rgba:- 2>/dev/null", finalPath.c_str());
    FILE *fpData = popen(cmd, "r");
    if (!fpData)
        return 0;

    size_t needed = pixels.size();
    size_t got = fread(&pixels[0], 1, needed, fpData);
    pclose(fpData);

    if (got != needed)
        return 0;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgW, imgH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

    return 1;
}

void loadBackground()
{
    if (!loadTexturePNG_UsingImageMagick("background.png", backgroundTex)) {
        std::cout << "Failed to load background.png" << std::endl;
    } else {
        std::cout << "Loaded background.png" << std::endl;
    }
}

void loadPlayerSprites()
{
    playerIdle.load("rifle/idle", 20);
    playerMove.load("rifle/move", 20);
    playerShoot.load("rifle/shoot", 3);

    // tune animation speed
    playerIdle.frameDelay  = 0.09f;
    playerMove.frameDelay  = 0.055f;
    playerShoot.frameDelay = 0.035f;

    // tune sprite size
    playerIdle.frameWidth = playerMove.frameWidth = playerShoot.frameWidth = 110;
    playerIdle.frameHeight = playerMove.frameHeight = playerShoot.frameHeight = 82;

    currentPlayerSprite = &playerIdle;
    spritesLoaded =
        (playerIdle.tex[0] != 0) &&
        (playerMove.tex[0] != 0) &&
        (playerShoot.tex[0] != 0);

    lastBulletStamp = bulletManager.bulletTimer;

    if (spritesLoaded) {
        std::cout << "Player sprites loaded." << std::endl;
    } else {
        std::cout << "Player sprites failed. Using old player render." << std::endl;
    }
}

void setCurrentPlayerSprite(Sprite *s)
{
    if (currentPlayerSprite != s) {
        currentPlayerSprite = s;
        if (currentPlayerSprite) {
            currentPlayerSprite->currentFrame = 0;
            currentPlayerSprite->frameTimer = 0.0f;
        }
    }
}

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
            PointerMotionMask | MotionNotify | ButtonPressMask | ButtonReleaseMask |
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
void renderBackground();
void renderMouseCrosshair();

int main()
{
    logOpen();
    init_opengl();

    loadBackground();
    loadPlayerSprites();

    initTitle();
    srand(time(NULL));
    zombie[0].init();
    nzombies = 1;
    clock_gettime(CLOCK_REALTIME, &zombieSpawnTimer);
    clock_gettime(CLOCK_REALTIME, &timePause);
    clock_gettime(CLOCK_REALTIME, &timeStart);

    int seconds = time(NULL);

    int done = 0;
    while (!done && !gl.done) {
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
        
        ++gl.nframes;
        int tmp = time(NULL);
        if (seconds != tmp) {
            gl.fps = gl.nframes;
            gl.nframes = 0;
            seconds = tmp;
        }

        x11.swapBuffers();
        //usleep(200);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initialize_fonts();
}

void check_mouse(XEvent *e)
{
    if (e->type == MotionNotify) {
        gl.mouse_x = e->xmotion.x;
        gl.mouse_y = gl.yres - e->xmotion.y;
        gl.mouse_y_down = e->xmotion.y; 
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button == 1) { 
            if (gl.state == STATE_TITLE) {
                checkTitleClick(e->xbutton.x, e->xbutton.y);
            }
        }
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
    if (gl.state == STATE_TITLE) {
        updateTitle((float)physicsRate);
        return;
    }
    
    player.update();

    // spawn a new zombie ever 1 sec up to MAX_ZOMBIES
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double elapsed = timeDiff(&zombieSpawnTimer, &now);
    if (elapsed >= 1.0 && nzombies < MAX_ZOMBIES) {
        zombie[nzombies].init();
        nzombies++;
        clock_gettime(CLOCK_REALTIME, &zombieSpawnTimer);
    }
    for (int i=0; i<nzombies; i++)
        zombie[i].update();
    
    
    checkCollisions();
    bulletManager.update(player);

    if (bulletTimerChanged(bulletManager.bulletTimer, lastBulletStamp)) {
        lastBulletStamp = bulletManager.bulletTimer;
        shootAnimTimer = 0.10;
        playerShoot.currentFrame = 0;
        playerShoot.frameTimer = 0.0f;
    }

    if (spritesLoaded) {
        if (shootAnimTimer > 0.0)
            shootAnimTimer -= physicsRate;
        if (shootAnimTimer < 0.0)
            shootAnimTimer = 0.0;

        if (shootAnimTimer > 0.0) {
            setCurrentPlayerSprite(&playerShoot);
        } else if (keysMoving()) {
            setCurrentPlayerSprite(&playerMove);
        } else {
            setCurrentPlayerSprite(&playerIdle);
        }

        if (currentPlayerSprite)
            currentPlayerSprite->update((float)physicsRate);
    }
}

void renderBackground()
{
    if (backgroundTex == 0)
        return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundTex);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f,      0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f,      gl.yres);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(gl.xres,   gl.yres);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(gl.xres,   0.0f);
    glEnd();
}

void renderMouseCrosshair()
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex2f(gl.mouse_x - 8, gl.mouse_y);
        glVertex2f(gl.mouse_x + 8, gl.mouse_y);
        glVertex2f(gl.mouse_x, gl.mouse_y - 8);
        glVertex2f(gl.mouse_x, gl.mouse_y + 8);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void render()
{
    if (gl.state == STATE_TITLE) {
        renderTitle();
        return;
    }

    glClearColor(0.0, 0.0, 0.0, 1.0); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    Rect r;
    renderBackground();

    r.bot = gl.yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint(&r, 16, 16, 0x00ffff00, "CMPS 4490 - Player/Zombie Test\n");
    ggprint(&r, 16, 16, 0x00ffffff, "FPS: %i\n", gl.fps);

    if (spritesLoaded && currentPlayerSprite) {
        float angleDegrees = player.angle * 180.0f / (float)PI;
        currentPlayerSprite->render(player.pos[0], player.pos[1], angleDegrees);
    } else {
        player.render();
    }

    glDisable(GL_TEXTURE_2D);
    for (int i=0; i<nzombies; i++)
        zombie[i].render();
    
    bulletManager.render();
    renderMouseCrosshair();
    glEnable(GL_TEXTURE_2D);
}
