#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>
#include "config.h"


static struct termios saved;

static void term_restore(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &saved);
    fputs("\033[?25h", stdout); /* show cursor */
}

static void term_raw(void) {
    tcgetattr(STDIN_FILENO, &saved);
    atexit(term_restore);
    struct termios r = saved;
    r.c_lflag &= ~(ECHO | ICANON);
    r.c_cc[VMIN] = 0; r.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &r);
}

#define KEY_UP   0x101
#define KEY_DOWN 0x102

static int readkey(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return 0;
    if (c == 3) return 'q';
    if (c != 27) return c;
    unsigned char s[2];
    if (read(STDIN_FILENO, &s[0], 1) != 1 || s[0] != '[') return 27;
    if (read(STDIN_FILENO, &s[1], 1) != 1) return 27;
    return s[1] == 'A' ? KEY_UP : s[1] == 'B' ? KEY_DOWN : 0;
}

static int pick(void) {
    int sel = 0;
    term_raw();
    fputs("\033[?25l", stdout); /* hide cursor */
    for (;;) {
        fputs("\033[2J\033[H", stdout);
        puts("\033[1;36m📻  radio\033[0m  \033[2m↑/↓ j/k move   Enter play   q quit\033[0m\n");
        for (int i = 0; i < N_STATIONS; i++) {
            if (i == sel)
                printf("\033[30;46m  ▶ %-16s  %s \033[0m\n", STATIONS[i].name, STATIONS[i].desc);
            else
                printf("    %-16s  \033[2m%s\033[0m\n",       STATIONS[i].name, STATIONS[i].desc);
        }
        fflush(stdout);
        int k = readkey();
        if (k == KEY_UP   || k == 'k') { if (sel > 0)             sel--; }
        if (k == KEY_DOWN || k == 'j') { if (sel < N_STATIONS-1)  sel++; }
        if (k == '\n' || k == '\r')    { term_restore(); fputs("\033[2J\033[H", stdout); return sel; }
        if (k == 'q')                  { term_restore(); fputs("\033[2J\033[H", stdout); return -1;  }
    }
}

static void play(const Station *s) {
    printf("\n\033[1;36m▶\033[0m \033[1m%s\033[0m  \033[2m%s\033[0m\n\n", s->name, s->desc);
    pid_t pid = fork();
    if (pid == 0) {
        execlp(PLAYER, PLAYER, "--no-video", "--really-quiet", s->url, NULL);
        perror(PLAYER); exit(1);
    }
    waitpid(pid, NULL, 0);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        int i = pick();
        if (i >= 0) play(&STATIONS[i]);
        return 0;
    }
    if (argc == 2) {
        for (int i = 0; i < N_STATIONS; i++)
            if (!strcmp(argv[1], STATIONS[i].name)) { play(&STATIONS[i]); return 0; }
        fprintf(stderr, "unknown station: %s\n", argv[1]);
        return 1;
    }
    puts("usage: radio           (pick interactively)\n"
         "       radio <name>    (play directly)");
    return 0;
}
