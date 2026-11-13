#include <stdio.h>
#include "tui.h"
#include "welt.h"
#include "uboot.h"

#define FPS 30
#define DT (1.0f / FPS)
#define SUB_LEN 3
#define SUB_SPRITE "=o>"

typedef enum { LAEUFT, VERLOREN, GEWONNEN } status;

typedef struct {
    welt start;          /* Welt beim Laden, fuer Neustart */
    welt w;               /* aktuelle Welt, Perlen verschwinden hier */
    uboot u;
    int perlen;           /* eingesammelt */
    int perlen_gesamt;    /* im Level vorhanden */
    status st;
} spiel;

static int perlen_zaehlen(const welt *w)
{
    int x, y, n = 0;
    for (y = 0; y < w->hoehe; y++) {
        for (x = 0; x < w->breite; x++) {
            if (welt_at(w, x, y) == '*') n++;
        }
    }
    return n;
}

static void spiel_neustart(spiel *s)
{
    s->w = s->start;                       /* Struct-Kopie: ganze Welt kopieren */
    s->u = uboot_start(1.0f, 4.5f, 8.0f, 0.0f);
    s->perlen = 0;
    s->st = LAEUFT;
}

static void spiel_schritt(spiel *s, float dt)
{
    int i;

    if (s->st != LAEUFT) return;

    uboot_bewegen(&s->u, &s->w, dt);

    for (i = 0; i < SUB_LEN; i++) {
        int cx = uboot_zelle_x(&s->u) + i;
        int cy = uboot_zelle_y(&s->u);
        char hier = welt_at(&s->w, cx, cy);

        if (hier == '#' || hier == 'M') {
            s->st = VERLOREN;
        } else if (hier == '*') {
            welt_setzen(&s->w, cx, cy, ' ');   /* Perle loeschen, nur in s->w! */
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

/* Haelt das U-Boot im linken Viertel des Bildschirms. */
static int kamera_x(int px, int breite)
{
    int cam_x = px - tui_width() / 4;
    if (cam_x > breite - tui_width()) cam_x = breite - tui_width();
    if (cam_x < 0) cam_x = 0;
    return cam_x;
}

static void welt_zeichnen(const welt *w, int cam_x, const uboot *u, status st)
{
    int x, y;
    for (y = 0; y < w->hoehe && y < tui_height() - 1; y++) {
        for (x = 0; x < tui_width() && cam_x + x < w->breite; x++) {
            char ch = welt_at(w, cam_x + x, y);
            if (ch != ' ') tui_put(x, y + 1, ch, farbe(ch));
        }
    }
    tui_text(uboot_zelle_x(u) - cam_x, uboot_zelle_y(u) + 1, SUB_SPRITE,
             st == VERLOREN ? TUI_COLOR_RED : TUI_COLOR_YELLOW);
}

static void eingabe_verarbeiten(spiel *s, int key, int *running)
{
    if (key == 'q') *running = 0;
    if (key == 'r') spiel_neustart(s);
    if (s->st != LAEUFT) return;

    if (key == TUI_KEY_UP || key == 'w') uboot_tauchen(&s->u, -1);
    if (key == TUI_KEY_DOWN || key == 's') uboot_tauchen(&s->u, 1);
    if (key == TUI_KEY_LEFT || key == 'a') uboot_tempo(&s->u, -1);
    if (key == TUI_KEY_RIGHT || key == 'd') uboot_tempo(&s->u, 1);
}

int main(int argc, char *argv[])
{
    const char *pfad = (argc > 1) ? argv[1] : "levels/aufgabe.txt";
    spiel s;
    int key, running = 1;
    char zeile[120];
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

        /* 1. Eingabe */
        while ((key = tui_poll_key()) != TUI_KEY_NONE) {
            eingabe_verarbeiten(&s, key, &running);
        }

        /* 2. Physik und Spielregeln */
        spiel_schritt(&s, DT);

        /* 3. Kamera */
        cam_x = kamera_x(uboot_zelle_x(&s.u), s.w.breite);

        /* 4. Zeichnen */
        tui_clear();
        welt_zeichnen(&s.w, cam_x, &s.u, s.st);

        snprintf(zeile, sizeof(zeile), "Perlen: %d/%d   %s",
                 s.perlen, s.perlen_gesamt,
                 s.st == GEWONNEN ? "Hafen erreicht! r fuer neue Fahrt"
                 : s.st == VERLOREN ? "Rumms. r fuer neue Fahrt" : "");
        tui_text(0, s.w.hoehe + 1, zeile, TUI_COLOR_CYAN);
        tui_present();

        /* 5. Warten */
        tui_sleep_ms(1000 / FPS);
    }

    tui_shutdown();
    printf("Perlen: %d von %d\n", s.perlen, s.perlen_gesamt);
    return 0;
}
