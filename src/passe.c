#include <stddef.h>
#include "spectre.h"
#define JIM_IMPLEMENTATION
#include "jim.h"
#define MJSON_IMPLEMENTATION
#include "mjson.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
#include "getopt_win32.h"
#else
#include <getopt.h>
#endif

static struct option long_options[] = {
    {NULL, 0, NULL, 0}
};

static void usage(void) {
    puts("usage: passe [options]\n");
}

int main(int argc, char *argv[]) {
    int opt;
    extern char* optarg;
    extern int optopt;
    while ((opt = getopt_long(argc, argv, "::", long_options, NULL)) != -1) {
        switch(opt) {
            case 'h':
                usage();
                return 0;
            case ':':
                break;
            case '?':
                break;
        }
    }
    return 0;
}
