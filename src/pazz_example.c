/* pazz_example.c -- https://github.com/takeiteasy/pazz

 The MIT License (MIT)

 Copyright (c) 2022 George Watson

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "pazz.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "getopt_win32.h"
#include <conio.h>
#else
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#endif

#define AUTHENTICATION "com.lyndir.masterpassword"
#define IDENTIFICATION "com.lyndir.masterpassword.login"
#define RECOVERY "com.lyndir.masterpassword.answer"

static struct option long_options[] = {
    {"name", required_argument, NULL, 'n'},
    {"password", required_argument, NULL, 'p'},
    {"site", required_argument, NULL, 's'},
    {"counter", required_argument, NULL, 'c'},
    {"scope", required_argument, NULL, 'S'},
    {"template", required_argument, NULL, 't'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

static void usage(void) {
    puts("usage: pazz --name username --site www.example.com [arg value...]");
    puts("");
    puts("  Arguments:");
    puts("    * --name/-n     -- Name of new user [required]");
    puts("    * --password/-p -- Master password of user [optional]");
    puts("                       If this is not set, you will be required");
    puts("                       to enter it through a masked prompt");
    puts("    * --site/-s     -- Site password identifier [required]");
    puts("    * --counter/-c  -- Modify to generate a different password");
    puts("                       for a site identifier [optional]");
    puts("    * --scope/-S    -- Scecify password purpose [optional]");
    puts("    * --template/-t -- Password output format [optional]");
    puts("");
    puts("  Scopes:");
    puts("    * authentication (default)");
    puts("    * identification");
    puts("    * recovery");
    puts("");
    puts("  Templates:");
    puts("    * max (20 characters)");
    puts("    * long (14 characters) (default)");
    puts("    * medium (8 characters)");
    puts("    * short (4 characters)");
    puts("    * basic (8 characters, A-Z + 0-9)");
    puts("    * pin (4 characters, 0-9)");
    puts("    * name (9 characters)");
    puts("    * phrase (20 characters with spaces)");
}

static const char* lowercase(const char *str) {
    static char lowercase[256];
    memset(&lowercase, 0, 256 * sizeof(char));
    for (size_t i = 0; i < strlen(str); i++)
        lowercase[i] = isalpha(str[i]) ? tolower(str[i]) : str[i];
    return lowercase;
}

#define ABORT(MSG, ...)                      \
    do {                                     \
        fprintf(stderr, (MSG), __VA_ARGS__); \
        usage();                             \
        return 1;                            \
    } while (0)

#define UNLESS(CND, MSG, ...)          \
    do {                               \
        if (!(CND)) {                  \
            ABORT((MSG), __VA_ARGS__); \
        }                              \
    } while(0)

#if !defined(_WIN32)
static void echo(int fd, int enabled) {
    struct termios old = {0};
    tcgetattr(fd, &old);
    if (!enabled)
        old.c_lflag &= ~ECHO;
    else
        old.c_lflag |= ECHO;
    tcsetattr(fd, TCSANOW, &old);
}
#endif

int main(int argc, char *argv[]) {
    int opt;
    extern char* optarg;
    extern int optopt;
    int counter = 1;
    const char *name = NULL,
               *password = NULL,
               *site = NULL,
               *scope = NULL,
               *template = NULL;
    while ((opt = getopt_long(argc, argv, ":h", long_options, NULL)) != -1) {
        switch(opt) {
            case 'n':
                name = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case 's':
                site = optarg;
                break;
            case 'c':
                counter = atoi(optarg);
                break;
            case 'S':
                scope = optarg;
                break;
            case 't':
                template = optarg;
                break;
            case 'h':
                usage();
                return 0;
            case ':':
                ABORT("ERROR: \"-%c\" requires an value!\n", optopt);
            case '?':
                ABORT("ERROR: Unknown argument \"-%c\"\n", optopt);
        }
    }

    UNLESS(name, "ERROR: %s\n", "No user name argument supplied");
    UNLESS(site, "ERROR: %s\n", "No site argument supplied");

    if (!scope)
        scope = AUTHENTICATION;
    else {
        const char *lower = lowercase(scope);
        if (!strncmp("authentication", lower, 14))
            scope = AUTHENTICATION;
        else if (!strncmp("identification", lower, 14))
            scope = IDENTIFICATION;
        else if (!strncmp("recovery", lower, 8))
            scope = RECOVERY;
        else
            ABORT("ERROR: Unknown scope \"%s\"", scope);
    }

    pazz_template_t spectre_template = TemplateLong;
    if (template) {
        const char *lower = lowercase(template);
        if (!strncmp("max", lower, 3) || !strncmp("maximum", lower, 7))
            spectre_template = TemplateMaximum;
        else if (!strncmp("long", lower, 4))
            spectre_template = TemplateLong;
        else if (!strncmp("medium", lower, 6))
            spectre_template = TemplateMedium;
        else if (!strncmp("short", lower, 5))
            spectre_template = TemplateShort;
        else if (!strncmp("pin", lower, 3))
            spectre_template = TemplatePin;
        else if (!strncmp("name", lower, 4))
            spectre_template = TemplateName;
        else if (!strncmp("phrase", lower, 6))
            spectre_template = TemplatePhrase;
        else
            ABORT("ERROR: Unknown template \"%s\"", template);
    }

    char input[256];
    if (!password) {
#if defined(_WIN32)
        int i = 0;
        for (;;) {
            // TODO: Bounds check + backspace
            char ch = _getch();
            if (ch == '\r') {
                input[i] = '\0';
                break;
            } else
                input[i++] = ch;
        }
#else
        echo(STDIN_FILENO, 0);
        fgets(input, sizeof(input), stdin);
        echo(STDIN_FILENO, 1);
        input[strcspn(input, "\n")] = 0;
#endif
        password = input;
    }

    printf("%s", spectre(name, password, site, counter, scope, spectre_template));
    return 0;
}
