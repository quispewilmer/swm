#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
/* X libs */
#include <X11/Xlib.h>

static Display *dpy;

void 
die (const char *strerr, ...) 
{
    va_list ap;
    va_start(ap, strerr);
    vfprintf(stderr, strerr, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

int 
main(int argc, char *argv[]) 
{
    if (argc == 2 && !strcmp("-v", argv[1]))
        die("swm-0.1\n");
    else if (argc != 1)
        die("swm: use swm [-v]\n");
    if (!(dpy = XOpenDisplay(NULL)))
        die("swm: cannot open display\n");
    /* TODO: Make top-level windows be shown to users */
}