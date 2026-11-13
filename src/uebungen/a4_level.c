#include <stdio.h>
#include "tui.h"
#include <string.h>

#define FPS 30
#define SUB_LEN 3
#define SUB_SPRITE "=o>"

#define W_MAX 400
#define H_MAX 40

static char welt[H_MAX][W_MAX];

/* Liest die Level-Datei pfad in welt.
 * Breite und Hoehe werden ueber die Pointer breite und hoehe zurueckgegeben.
 * Rueckgabe: 0 bei Erfolg, -1 bei Fehler. */
static int welt_laden(const char *pfad, int *breite, int *hoehe)
{
    char zeile[W_MAX + 2];
    FILE *f;

    f = fopen(pfad, "r");
    if (f == NULL) return -1;

    *breite = 0;
    *hoehe = 0;

    while (*hoehe < H_MAX && fgets(zeile, (int)sizeof(zeile), f) != NULL) {
        int x;
        int len = (int)strlen(zeile);

        while (len > 0 && (zeile[len - 1] == '\n' || zeile[len - 1] == '\r')) {
            zeile[--len] = '\0';
        }
        if (len > W_MAX) len = W_MAX;

        for (x = 0; x < W_MAX; x++) {
            welt[*hoehe][x] = (x < len) ? zeile[x] : ' ';
        }
        if (len > *breite) *breite = len;

        (*hoehe)++;
    }

    fclose(f);
    return 0;
}

static char welt_at(int x, int y, int breite, int hoehe)
{
    if (y < 0) return '~';                            /* oberhalb: Oberfläche */
    if (x < 0 || x >= breite || y >= hoehe) return '#'; /* sonst ringsum Fels   */
    return welt[y][x];
}

static int frei(int x, int y, int breite, int hoehe)
{
    int i;
    for (i = 0; i < SUB_LEN; i++) {
        if (welt_at(x + i, y, breite, hoehe) != ' ') return 0;
    }
    return 1;   /* Schleife ohne Fund durchgelaufen: alles frei */
}

int main(int argc, char *argv[])
{
    const char *pfad = (argc > 1) ? argv[1] : "levels/aufgabe.txt";
    int breite, hoehe;
    int x = 10;          /* Spalte des Hecks */
    int y = 5;           /* Zeile des Hecks */
    int key;
    int laeuft = 1;      /* 1 = Spiel laeuft, 0 = beenden */
    char status[80];
    int wx, wy;
    int nx, ny;
    int cam_x;

    if (welt_laden(pfad, &breite, &hoehe) != 0) {
        fprintf(stderr, "Fehler beim Laden von %s\n", pfad);
        return 1;
    }

    if (tui_init() != 0) {
        fprintf(stderr, "Fehler beim Initialisieren des TUI\n");
        return 1;
    }

    while (laeuft) {

        /* 1. Eingabe: alle anliegenden Tasten lesen */
        nx = x;
        ny = y;

        while ((key = tui_poll_key()) != TUI_KEY_NONE) {
            switch (key) {
                case TUI_KEY_LEFT:
                    nx--;
                    break;
                case TUI_KEY_RIGHT:
                    nx++;
                    break;
                case TUI_KEY_UP:
                    ny--;
                    break;
                case TUI_KEY_DOWN:
                    ny++;
                    break;
                case 'q':
                    laeuft = 0;
                    break;
            }
        }

        /* 2. Regeln: das ganze U-Boot bleibt im Bild */
        if (frei(nx, (ny - 1), breite, hoehe) == 1) {
            x = nx;
            y = ny;
        }

        /* 3. Kamera: U-Boot im linken Viertel halten */
        cam_x = x - tui_width() / 4;
        if (cam_x > breite - tui_width()) cam_x = breite - tui_width();
        if (cam_x < 0) cam_x = 0;

        /* 4. Zeichnen */
        tui_clear();

        for (wy = 0; wy < hoehe && wy < tui_height() - 1; wy++) {
            for (wx = 0; wx < tui_width() && cam_x + wx < breite; wx++) {
                char ch = welt[wy][cam_x + wx];
                if (ch == '#') {
                    tui_put(wx, wy + 1, ch, TUI_COLOR_GREEN);
                }
                if (ch == '~') {
                    tui_put(wx, wy + 1, ch, TUI_COLOR_CYAN);
                }
                if (ch == '*') {
                    tui_put(wx, wy + 1, ch, TUI_COLOR_WHITE);
                }
                if (ch == 'M') {
                    tui_put(wx, wy + 1, ch, TUI_COLOR_RED);
                }
                if (ch == 'F') {
                    tui_put(wx, wy + 1, ch, TUI_COLOR_MAGENTA);
                }
            }
        }
        snprintf(status, sizeof(status), "x=%d y=%d   (q = beenden)", x, y);
        tui_text(0, 0, status, TUI_COLOR_WHITE);
        tui_text(x - cam_x, y, SUB_SPRITE, TUI_COLOR_YELLOW);
        tui_present();

        /* 5. Warten */
        tui_sleep_ms(1000 / FPS);
    }

    tui_shutdown();
    return 0;
}
