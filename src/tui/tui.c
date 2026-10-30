#include "tui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>

/* ------------------------------------------------------------------ Zustand */

/* Der Bildpuffer: fuer jede Position ein Zeichen und eine Farbe.
 * Zwei 2D-Arrays, genau wie die Spielwelt im Spiel selbst. */
static char      g_chars[TUI_MAX_H][TUI_MAX_W];
static unsigned char g_colors[TUI_MAX_H][TUI_MAX_W];

static int g_width = 80;
static int g_height = 24;

static struct termios g_saved_termios; /* Terminal-Einstellungen vor tui_init */
static int g_active = 0;               /* 1, wenn der Roh-Modus aktiv ist */

/* ANSI-Sequenzen fuer die Vordergrundfarben, Index = tui_color */
static const char *const g_color_seq[] = {
    "\033[0m",  /* default */
    "\033[31m", /* rot */
    "\033[32m", /* gruen */
    "\033[33m", /* gelb */
    "\033[34m", /* blau */
    "\033[35m", /* magenta */
    "\033[36m", /* cyan */
    "\033[37m"  /* weiss */
};

/* ------------------------------------------------------------- Hilfsfunktionen */

static void tui_query_size(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        g_width = ws.ws_col;
        g_height = ws.ws_row;
    }
    if (g_width > TUI_MAX_W) g_width = TUI_MAX_W;
    if (g_height > TUI_MAX_H) g_height = TUI_MAX_H;
}

static void tui_on_signal(int sig)
{
    (void)sig;
    tui_shutdown();
    _exit(1); /* im Signal-Handler nur async-signal-sichere Funktionen */
}

/* ------------------------------------------------------------ oeffentliche API */

int tui_init(void)
{
    struct termios raw;

    if (!isatty(STDOUT_FILENO) || !isatty(STDIN_FILENO)) {
        return -1; /* keine Terminalausgabe, z.B. Ausgabe in eine Datei */
    }
    if (tcgetattr(STDIN_FILENO, &g_saved_termios) == -1) {
        return -1;
    }

    raw = g_saved_termios;
    /* ICANON aus: Zeichen kommen sofort an, nicht erst nach Enter.
     * ECHO aus: Tastendruecke werden nicht mehr angezeigt. */
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;  /* read() wartet nicht auf ein Zeichen ... */
    raw.c_cc[VTIME] = 0; /* ... und auch nicht auf Zeit: sofort zurueck */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return -1;
    }

    g_active = 1;
    atexit(tui_shutdown);          /* aufraeumen bei normalem Ende */
    signal(SIGINT, tui_on_signal); /* aufraeumen bei Strg-C */
    signal(SIGTERM, tui_on_signal);

    /* Alternativer Bildschirm, damit das Terminal danach wieder so aussieht
     * wie vorher, und Cursor ausblenden. */
    if (write(STDOUT_FILENO, "\033[?1049h\033[?25l", 14) < 0) { /* ignorieren */ }

    tui_query_size();
    tui_clear();
    return 0;
}

void tui_shutdown(void)
{
    if (!g_active) return;
    g_active = 0;
    if (write(STDOUT_FILENO, "\033[0m\033[?25h\033[?1049l", 18) < 0) { /* ignorieren */ }
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved_termios);
}

int tui_width(void) { return g_width; }
int tui_height(void) { return g_height; }

void tui_clear(void)
{
    int y, x;
    tui_query_size(); /* Fenstergroesse kann sich geaendert haben */
    for (y = 0; y < g_height; y++) {
        for (x = 0; x < g_width; x++) {
            g_chars[y][x] = ' ';
            g_colors[y][x] = TUI_COLOR_DEFAULT;
        }
    }
}

void tui_put(int x, int y, char ch, tui_color color)
{
    if (x < 0 || y < 0 || x >= g_width || y >= g_height) return;
    g_chars[y][x] = ch;
    g_colors[y][x] = (unsigned char)color;
}

void tui_text(int x, int y, const char *s, tui_color color)
{
    int i;
    if (s == NULL) return;
    for (i = 0; s[i] != '\0'; i++) {
        tui_put(x + i, y, s[i], color);
    }
}

void tui_present(void)
{
    /* Das ganze Bild wird in einem Puffer zusammengebaut und mit einem
     * einzigen write() ausgegeben. Wuerde man Zeichen fuer Zeichen schreiben,
     * flackert das Bild. */
    static char out[TUI_MAX_H * TUI_MAX_W * 8 + 64];
    size_t len = 0;
    int y, x;
    int current = -1;

    /* Cursor nach oben links */
    memcpy(out + len, "\033[H", 3);
    len += 3;

    for (y = 0; y < g_height; y++) {
        for (x = 0; x < g_width; x++) {
            int c = g_colors[y][x];
            if (c != current) {
                size_t n = strlen(g_color_seq[c]);
                memcpy(out + len, g_color_seq[c], n);
                len += n;
                current = c;
            }
            out[len++] = g_chars[y][x];
        }
        if (y + 1 < g_height) {
            memcpy(out + len, "\r\n", 2);
            len += 2;
        }
    }
    memcpy(out + len, "\033[0m", 4);
    len += 4;

    if (write(STDOUT_FILENO, out, len) < 0) { /* ignorieren */ }
}

int tui_poll_key(void)
{
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n != 1) return TUI_KEY_NONE;

    if (c == 27) { /* ESC: entweder allein oder Beginn einer Pfeiltasten-Sequenz */
        unsigned char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return TUI_KEY_ESC;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return TUI_KEY_ESC;
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return TUI_KEY_UP;
                case 'B': return TUI_KEY_DOWN;
                case 'C': return TUI_KEY_RIGHT;
                case 'D': return TUI_KEY_LEFT;
                default: return TUI_KEY_ESC;
            }
        }
        return TUI_KEY_ESC;
    }
    return (int)c;
}

long tui_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

void tui_sleep_ms(int ms)
{
    struct timespec ts;
    if (ms <= 0) return;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}