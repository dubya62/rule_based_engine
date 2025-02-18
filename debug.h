
#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1
#define DBG(...) if(DEBUG){ printf(__VA_ARGS__);}
#define PANIC(...) {fprintf(stderr, __VA_ARGS__); exit(1);}

#endif
