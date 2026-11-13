 #include <stdio.h>
#include "tui.h"

#define FPS 30
#define SUB_LEN 3
#define SUB_SPRITE "=o>"

int main(void)
{
    int x = 10;          /* Spalte des Hecks */
    int y = 5;           /* Zeile des Hecks */
    int key;
    int laeuft = 1;      /* 1 = Spiel laeuft, 0 = beenden */
    char status[80];

    if (tui_init() != 0) {
        fprintf(stderr, "Fehler beim Initialisieren des TUI\n");
        return 1;
    }

    while (laeuft) {

        /* 1. Eingabe: alle anliegenden Tasten lesen */
        while ((key = tui_poll_key()) != TUI_KEY_NONE) {
            switch (key) {
                case TUI_KEY_LEFT:
                    x--;
                    break;
                case TUI_KEY_RIGHT:
                    x++;
                    break;
                case TUI_KEY_UP:
                    y--;
                    break;
                case TUI_KEY_DOWN:
                    y++;
                    break;
                case 'q':
                    laeuft = 0;
                    break;
            }
        }

        /* 2. Regeln: das ganze U-Boot bleibt im Bild */
        if (x < 0) x = 0;
        if (x > tui_width() - SUB_LEN) x = tui_width() - SUB_LEN;
        if (y < 1) y = 1;                       /* Zeile 0 ist die Statuszeile */
        if (y > tui_height() - 1) y = tui_height() - 1;

        /* 3. Zeichnen */
        tui_clear();
        snprintf(status, sizeof(status), "x=%d y=%d   (q = beenden)", x, y);
        tui_text(0, 0, status, TUI_COLOR_WHITE);
        tui_text(x, y, SUB_SPRITE, TUI_COLOR_YELLOW);
        tui_present();

        /* 4. Warten */
        tui_sleep_ms(1000 / FPS);
    }

    tui_shutdown();
    return 0;
}