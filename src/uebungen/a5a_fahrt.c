#include <stdio.h>
#include "tui.h"
#include <string.h>
#include <math.h>

#define FPS 30
#define DT (1.0f / FPS)
#define SUB_LEN 3
#define SUB_SPRITE "=o>"

#define W_MAX 400
#define H_MAX 40

#define SCHUB 4.0f        /* Schub bei Tastendruck */
#define DRAG 2.0f         /* Wasserwiderstand */
#define TEMPO_START 8.0f  /* Starttempo in Zellen/Sekunde */
#define VX_MIN 2.0f
#define VX_MAX 16.0f

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    char zellen[H_MAX][W_MAX];
    int breite;
    int hoehe;
} welt;

typedef struct {
    float x;    /* linke Zelle (Heck) */
    float y;
    float vx;   /* Zellen pro Sekunde, nach rechts */
    float vy;   /* Zellen pro Sekunde, positiv heisst nach unten */
} uboot;

/* Liest die Level-Datei pfad in w. Rueckgabe: 0 bei Erfolg, -1 bei Fehler. */
static int welt_laden(welt *w, const char *pfad)
{
    char zeile[W_MAX + 2];
    FILE *f;

    f = fopen(pfad, "r");
    if (f == NULL) return -1;

    w->breite = 0;
    w->hoehe = 0;

    while (w->hoehe < H_MAX && fgets(zeile, (int)sizeof(zeile), f) != NULL) {
        int x;
        int len = (int)strlen(zeile);

        while (len > 0 && (zeile[len - 1] == '\n' || zeile[len - 1] == '\r')) {
            zeile[--len] = '\0';
        }
        if (len > W_MAX) len = W_MAX;

        for (x = 0; x < W_MAX; x++) {
            w->zellen[w->hoehe][x] = (x < len) ? zeile[x] : ' ';
        }
        if (len > w->breite) w->breite = len;

        w->hoehe++;
    }

    fclose(f);
    return 0;
}

static char welt_at(const welt *w, int x, int y)
{
    if (y < 0) return '~';                                 /* oberhalb: Oberfläche */
    if (x < 0 || x >= w->breite || y >= w->hoehe) return '#'; /* sonst ringsum Fels */
    return w->zellen[y][x];
}

static int frei(const welt *w, int x, int y)
{
    int i;
    for (i = 0; i < SUB_LEN; i++) {
        if (welt_at(w, x + i, y) != ' ') return 0;
    }
    return 1;
}

static int zelle(float v)
{
    return (int)floorf(v); /* floorf statt (int)v: rundet auch bei negativen Werten korrekt ab */
}

static void uboot_steuern(uboot *u, int key)
{
    if (key == TUI_KEY_UP || key == 'w') u->vy -= SCHUB;
    if (key == TUI_KEY_DOWN || key == 's') u->vy += SCHUB;
    if (key == TUI_KEY_LEFT || key == 'a') u->vx = MAX(VX_MIN, u->vx - 2.0f);
    if (key == TUI_KEY_RIGHT || key == 'd') u->vx = MIN(VX_MAX, u->vx + 2.0f);
}

static void uboot_bewegen(uboot *u, const welt *w, float dt)
{
    float nx, ny;

    /* Waagerecht: Bewegung mit konstantem Tempo */
    nx = u->x + u->vx * dt;
    if (frei(w, zelle(nx), zelle(u->y))) u->x = nx;

    /* Senkrecht: Wasserwiderstand bremst vy weich ab */
    u->vy -= u->vy * DRAG * dt;
    ny = u->y + u->vy * dt;
    if (frei(w, zelle(u->x), zelle(ny))) u->y = ny;
    else u->vy = 0.0f;
}

int main(int argc, char *argv[])
{
    const char *pfad = (argc > 1) ? argv[1] : "levels/aufgabe.txt";
    welt w;
    uboot u;
    int key;
    int laeuft = 1;
    char status[80];
    int wx, wy;
    int cam_x;

    if (welt_laden(&w, pfad) != 0) {
        fprintf(stderr, "Fehler beim Laden von %s\n", pfad);
        return 1;
    }

    u = (uboot){1.0f, 4.5f, TEMPO_START, 0.0f};

    if (tui_init() != 0) {
        fprintf(stderr, "Fehler beim Initialisieren des TUI\n");
        return 1;
    }

    while (laeuft) {

        /* 1. Eingabe: alle anliegenden Tasten lesen */
        while ((key = tui_poll_key()) != TUI_KEY_NONE) {
            if (key == 'q') {
                laeuft = 0;
                continue;
            }
            uboot_steuern(&u, key);
        }

        /* 2. Physik: Schub, Widerstand, Kollision */
        uboot_bewegen(&u, &w, DT);

        /* 3. Kamera: U-Boot im linken Viertel halten */
        cam_x = zelle(u.x) - tui_width() / 4;
        if (cam_x > w.breite - tui_width()) cam_x = w.breite - tui_width();
        if (cam_x < 0) cam_x = 0;

        /* 4. Zeichnen */
        tui_clear();

        for (wy = 0; wy < w.hoehe && wy < tui_height() - 1; wy++) {
            for (wx = 0; wx < tui_width() && cam_x + wx < w.breite; wx++) {
                char ch = w.zellen[wy][cam_x + wx];
                if (ch == '#') tui_put(wx, wy + 1, ch, TUI_COLOR_GREEN);
                if (ch == '~') tui_put(wx, wy + 1, ch, TUI_COLOR_CYAN);
                if (ch == '*') tui_put(wx, wy + 1, ch, TUI_COLOR_WHITE);
                if (ch == 'M') tui_put(wx, wy + 1, ch, TUI_COLOR_RED);
                if (ch == 'F') tui_put(wx, wy + 1, ch, TUI_COLOR_MAGENTA);
            }
        }
        snprintf(status, sizeof(status), "x=%.1f y=%.1f vx=%.1f vy=%.1f   (q = beenden)",
                 u.x, u.y, u.vx, u.vy);
        tui_text(0, 0, status, TUI_COLOR_WHITE);
        tui_text(zelle(u.x) - cam_x, zelle(u.y) + 1, SUB_SPRITE, TUI_COLOR_YELLOW);
        tui_present();

        /* 5. Warten */
        tui_sleep_ms(1000 / FPS);
    }

    tui_shutdown();
    return 0;
}
