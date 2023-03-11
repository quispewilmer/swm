#include <stdio.h>
#include <string.h> 
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
/* X libs */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/*
 * Xorg (X) provides an X Server through which the Window Manager (WM) can make 
 * requests using client-server model. These requests are usually mouse or 
 * keyboard taps. The WM is no more than another program which the X Server has 
 * to interact with, but the WM can access to APIs that common applications 
 * don't use and don't need to. Thanks to the client-server model, the X server 
 * can run into the same computer as the WM or even in another computer that 
 * has a way to connect with the client computer. The communication in the 
 * first case (same computer) is using sockets and in the second (server and 
 * client computers) is using TCP/IP protocol.
 * 
 * ┌──────────────┐ ┌─────────────┐ ┌─────────────┐
 * │Window Manager│ │Application X│ │Application Y│
 * └──────┬───────┘ └──────┬──────┘ └──────┬──────┘
 *        │ ▲              │ ▲             │ ▲
 *        ▼ │              ▼ │             ▼ │
 * ┌────────┴────────────────┴───────────────┴────┐
 * │                   X Server                   │
 * └───────────┬──────────────────────────────────┘
 *             │ ▲         ▲          ▲
 *             ▼ │         │          │
 *        ┌──────┴───┐ ┌───┴───┐ ┌────┴─────┐
 *        │Video Card│ │ Mouse │ │ Keyboard │
 *        └──────────┘ └───────┘ └──────────┘
 */

static Display *dpy;
static Window root;
static Screen *screen;
static int screen_num;
static int dw;
static int dh;
static Bool is_another_wm;
static Bool is_running = True;

static void die (const char *, ...);
static void setup ();
static void check_other_wm ();
static int on_wm_detected (Display *, XErrorEvent *);
static int x_error (Display *, XErrorEvent *);
static void run ();
static void on_create_notify (XEvent *);
static void on_destroy_notify (XEvent *);
static void on_map_notify (XEvent *);
static void on_reparent_notify (XEvent *); 
static void on_configure_request (XEvent *);
static void on_map_request (XEvent *);
static void on_unmap_notify (XEvent *);
static void on_button_press (XEvent *);
static void on_button_release (XEvent *);
static void on_motion_notify (XEvent *);
static void on_key_press (XEvent *);
static void on_key_release (XEvent *);
static void manage_request (Window, XWindowAttributes *);
static void unmanage_request (Window);
static void spawn ();

void 
die (const char *strerr, ...) 
{
    va_list ap;
    va_start(ap, strerr);
    vfprintf(stderr, strerr, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void
check_other_wm () 
{
    is_another_wm = False;
    XSetErrorHandler(on_wm_detected);
    /* Returns error if a window manager is currently running */
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask|SubstructureNotifyMask);
    XSync(dpy, False);
    if (is_another_wm)
        die("swm: there's another window manager running\n");
    XSetErrorHandler(x_error);
    XSync(dpy, False);
}

int
on_wm_detected (Display *dpy, XErrorEvent *e) 
{
    is_another_wm = True;
    return -1;
}

int
x_error (Display *dpy, XErrorEvent *e) 
{
    fprintf(stderr, "swm: fatal error, code=%d\n", e->error_code);
}

void
setup () 
{
    root = DefaultRootWindow(dpy);
    screen = DefaultScreenOfDisplay(dpy);
    screen_num = DefaultScreen(dpy);
    dw = DisplayWidth(dpy, screen_num);
    dh = DisplayHeight(dpy, screen_num);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Return), Mod1Mask, root, False, GrabModeAsync, GrabModeAsync);
}

void
run () 
{
    XEvent e;
    XSync(dpy, False);
    while (is_running && !XNextEvent(dpy, &e)) {
        switch(e.type) {
            case CreateNotify:
                on_create_notify(&e);
                break;
            case DestroyNotify:
                on_destroy_notify(&e);
                break;
            case MapNotify:
                on_map_notify(&e);
                break;
            case ReparentNotify:
                on_reparent_notify(&e);
                break;
            case ConfigureRequest:
                on_configure_request(&e);
                break;
            case MapRequest:
                on_map_request(&e);
                break;
            case UnmapNotify:
                on_unmap_notify(&e);
                break;
            case ButtonPress:
                on_button_press(&e);
                break;
            case ButtonRelease:
                on_button_release(&e);
                break;
            case MotionNotify:
                on_motion_notify(&e);
                break;
            case KeyPress:
                on_key_press(&e);
                break;
            case KeyRelease:
                on_key_release(&e);
                break;
            default:
                fprintf(stdout, "swm: unknown event\n");
                break;
        }
    }
}

void
on_create_notify (XEvent *e) 
{ }

void
on_destroy_notify (XEvent *e) 
{ }

void
on_map_notify (XEvent *e) 
{ }

void
on_reparent_notify (XEvent *e) 
{ }

void
on_configure_request (XEvent *e) 
{
    XConfigureRequestEvent cre = e->xconfigurerequest;
    XWindowChanges wc;
    wc.x = cre.x;
    wc.y = cre.y;
    wc.width = cre.width;
    wc.height = cre.height;
    wc.border_width = cre.border_width;
    wc.stack_mode = cre.detail;
    wc.sibling = cre.above;
    XConfigureWindow(dpy, cre.window, cre.value_mask, &wc);
    XSync(dpy, False);
}

void
on_map_request (XEvent *e) 
{
    XMapRequestEvent mre = e->xmaprequest;
    static XWindowAttributes wa;
    XGetWindowAttributes(dpy, mre.window, &wa);
    manage_request(mre.window, &wa);
    XMapWindow(dpy, mre.window);
}

void
manage_request (Window w, XWindowAttributes *wa) 
{
    const Window frame = XCreateSimpleWindow(dpy, root, wa->x, wa->y, wa->width, wa->height, 3, 0xff0000, 0x00000000);
    XGrabButton(dpy, Button1, Mod4Mask, w, False, ButtonPressMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, Button3, Mod4Mask, w, False, ButtonPressMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_F4), Mod1Mask, w, False, GrabModeAsync, GrabModeAsync);
    XAddToSaveSet(dpy, w);
    XReparentWindow(dpy, w, frame, 0, 0);
    XMapWindow(dpy, frame);
}

void
on_unmap_notify (XEvent *e) 
{
    XUnmapEvent ue = e->xunmap;
    unmanage_request(ue.window);
}

void
unmanage_request (Window w) 
{
    XUnmapWindow(dpy, w);
    XReparentWindow(dpy, w, root, 0, 0);
    XRemoveFromSaveSet(dpy, w);
    XDestroyWindow(dpy, w);
}

void
on_button_press (XEvent *e) 
{

}

void
on_button_release (XEvent *e) 
{ }

void
on_motion_notify (XEvent *e) 
{

}

void
on_key_press (XEvent *e) 
{
    XKeyEvent ke = e->xkey;
    if (ke.keycode == XKeysymToKeycode(dpy, XK_F4))
        printf("Closing window...");
    else if (ke.keycode == XKeysymToKeycode(dpy, XK_Return))
        spawn();
}

void
on_key_release (XEvent *e) 
{ }

void
spawn () 
{
    execvp("xterm", NULL);
}

int 
main(int argc, char *argv[]) 
{
    if (argc == 2 && !strcmp("-v", argv[1]))
        die("swm-0.1\n");
    else if (argc != 1)
        die("swm: use swm [-v]\n");
    if (!XSupportsLocale)
        die("swm: there's no locale support for Xlib functions\n");
    if (!(dpy = XOpenDisplay((char *) NULL)))
        die("swm: cannot open X display\n");
    fprintf(stdout, "swm: number of connection with x server: %d\n", ConnectionNumber(dpy));
    check_other_wm();
    setup();
    run();
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}