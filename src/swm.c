#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

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
        die("swm 0.1");
    else if (argc != 1)
        die("use: swm [-v]");
    
}