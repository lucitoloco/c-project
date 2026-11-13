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

#define DRAG 2.0f
#define TEMPO_START 8.0f
#define SCHUB 4.0f
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

typedef enum { LAEUFT, VERLOREN, GEWONNEN } status;

typedef struct {
    welt start;         /* Welt beim Laden, fuer Neustart */
    welt w;              /* aktuelle Welt, Perlen verschwinden hier */
    uboot u;
    int perlen;          /* eingesammelt */
    int perlen_gesamt;   /* im Level vorhanden */
    status st;
} spiel;

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
    if (y < 0) return '~';                                    /* oberhalb: Oberfläche */
    if (x < 0 || x >= w->breite || y >= w->hoehe) return '#';  /* sonst ringsum Fels */
    return w->zellen[y][x];
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

static int perlen_zaehlen(const welt *w)
{
    int x, y, n = 0;
    for (y = 0; y < w->hoehe; y++) {
        for (x = 0; x < w->breite; x++) {
            if (w->zellen[y][x] == '*') n++;
        }
    }
    return n;
}

/* Setzt die laufende Welt auf den Ladezustand zurueck (Perlen kommen wieder),
 * und das U-Boot auf die Startwerte. */
static void spiel_neustart(spiel *s)
{
    s->w = s->start;                                 /* Struct-Kopie: ganze Welt kopieren */
    s->u = (uboot){1.0f, 4.5f, TEMPO_START, 0.0f};
    s->perlen = 0;
    s->st = LAEUFT;
}

/* Bewegt das U-Boot einen Zeitschritt weiter und prueft die Spielregeln:
 * Fels/Mine beendet die Fahrt, Perlen werden eingesammelt, Hafen gewinnt. */
static void spiel_schritt(spiel *s, float dt)
{
    uboot *u = &s->u;
    int i, an_oberflaeche;
    float ny;

    if (s->st != LAEUFT) return;

    /* a) waagerecht: ungeprueft, Felskontakt wird unten in (c) erkannt */
    u->x += u->vx * dt;

    /* b) senkrecht: Wasserwiderstand, Wasseroberflaeche haelt auf */
    u->vy -= u->vy * DRAG * dt;
    ny = u->y + u->vy * dt;
    an_oberflaeche = 0;
    for (i = 0; i < SUB_LEN; i++) {
        if (welt_at(&s->w, zelle(u->x) + i, zelle(ny)) == '~') an_oberflaeche = 1;
    }
    if (an_oberflaeche) u->vy = 0.0f;
    else u->y = ny;

    /* c) Regeln fuer jede Zelle des U-Boots */
    for (i = 0; i < SUB_LEN; i++) {
        int cx = zelle(u->x) + i;
        int cy = zelle(u->y);
        char hier = welt_at(&s->w, cx, cy);

        if (hier == '#' || hier == 'M') {
            s->st = VERLOREN;
        } else if (hier == '*') {
            s->w.zellen[cy][cx] = ' ';  /* Perle loeschen, nur in s->w! */
            s->perlen++;
        } else if (hier == 'F') {
            s->st = GEWONNEN;
        }
    }
}

static tui_color farbe(char ch)
{
    switch (ch) {
        case '#': return TUI_COLOR_GREEN;
        case '~': return TUI_COLOR_CYAN;
        case '*': return TUI_COLOR_WHITE;
        case 'M': return TUI_COLOR_RED;
        case 'F': return TUI_COLOR_MAGENTA;
        default:  return TUI_COLOR_DEFAULT;
    }
}

int main(int argc, char *argv[])
{
    const char *pfad = (argc > 1) ? argv[1] : "levels/aufgabe.txt";
    spiel s;
    int key;
    int running = 1;
    char zeile[120];
    int wx, wy;
    int cam_x;

    if (welt_laden(&s.start, pfad) != 0) {
        fprintf(stderr, "Fehler beim Laden von %s\n", pfad);
        return 1;
    }
    s.perlen_gesamt = perlen_zaehlen(&s.start);
    spiel_neustart(&s);

    if (tui_init() != 0) {
        fprintf(stderr, "Fehler beim Initialisieren des TUI\n");
        return 1;
    }

    while (running) {

        /* 1. Eingabe: alle anliegenden Tasten lesen */
        while ((key = tui_poll_key()) != TUI_KEY_NONE) {
            if (key == 'q') running = 0;
            if (key == 'r') spiel_neustart(&s);
            if (s.st == LAEUFT) uboot_steuern(&s.u, key);
        }

        /* 2. Physik und Spielregeln */
        spiel_schritt(&s, DT);

        /* 3. Kamera: U-Boot im linken Viertel halten */
        cam_x = zelle(s.u.x) - tui_width() / 4;
        if (cam_x > s.w.breite - tui_width()) cam_x = s.w.breite - tui_width();
        if (cam_x < 0) cam_x = 0;

        /* 4. Zeichnen */
        tui_clear();

        for (wy = 0; wy < s.w.hoehe && wy < tui_height() - 1; wy++) {
            for (wx = 0; wx < tui_width() && cam_x + wx < s.w.breite; wx++) {
                char ch = s.w.zellen[wy][cam_x + wx];
                if (ch != ' ') tui_put(wx, wy + 1, ch, farbe(ch));
            }
        }

        snprintf(zeile, sizeof(zeile), "Perlen: %d/%d   %s",
                 s.perlen, s.perlen_gesamt,
                 s.st == GEWONNEN ? "Hafen erreicht! r fuer neue Fahrt"
                 : s.st == VERLOREN ? "Rumms. r fuer neue Fahrt" : "");
        tui_text(0, s.w.hoehe + 1, zeile, TUI_COLOR_CYAN);
        tui_text(zelle(s.u.x) - cam_x, zelle(s.u.y) + 1, SUB_SPRITE,
                 s.st == VERLOREN ? TUI_COLOR_RED : TUI_COLOR_YELLOW);
        tui_present();

        /* 5. Warten */
        tui_sleep_ms(1000 / FPS);
    }

    tui_shutdown();
    printf("Perlen: %d von %d\n", s.perlen, s.perlen_gesamt);
    return 0;
}
