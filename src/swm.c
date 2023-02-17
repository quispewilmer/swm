#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
/* X libs */
#include <X11/Xlib.h>

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

void die (const char *, ...);

int 
main(int argc, char *argv[]) 
{
    if (argc == 2 && !strcmp("-v", argv[1]))
        die("swm-0.1\n");
    else if (argc != 1)
        die("swm: use swm [-v]\n");
    if (!XSupportsLocale)
        die("swm: there's no locale support\n");
    if (!(dpy = XOpenDisplay(NULL)))
        die("swm: cannot open display\n");
    XCloseDisplay(dpy);
}

void 
die (const char *strerr, ...) 
{
    va_list ap;
    va_start(ap, strerr);
    vfprintf(stderr, strerr, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}