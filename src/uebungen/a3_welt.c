#include <stdio.h>
#include "tui.h"
#include <string.h>

#define FPS 30
#define SUB_LEN 3
#define SUB_SPRITE "=o>"

#define W 60
#define H 10
static const char *const level[H] = {
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",
    "                    ###               ######               #",
    "                    ###                ####                #",
    "                     #                  ##                 #",
    "                                                           #",
    "                                                  ###      #",
    "            ###                                  #####     #",
    "           #####         ###                    #######    #",
    "   ###    #######       #####         ####     #########   #",
    "############################################################"
};

static char welt[H][W];

static void welt_laden(void)
{
    int x, y;
    for (y = 0; y < H; y++) {
        int len = (int)strlen(level[y]);
        for (x = 0; x < W; x++) {
            if (x < len){
                welt[y][x] = level[y] [x];
            }else {
                  welt[y][x] = ' ' ;

            }
        }
    }      
}

static char welt_at(int x, int y)
{
    if (y < 0) return '~';                       /* oberhalb: Oberfläche */
    if (x < 0 || x >= W || y >= H) return '#';   /* sonst ringsum Fels   */
    return welt[y][x];
}


static int frei(int x, int y)
{
    int i;
    for (i = 0; i < SUB_LEN; i++) {
        if (welt_at (x+i ,y) !=  ' ') return 0;
    }
    return 1;   /* Schleife ohne Fund durchgelaufen: alles frei */
}

int main(void)
{
    int x = 10;          /* Spalte des Hecks */
    int y = 5;           /* Zeile des Hecks */
    int key;
    int laeuft = 1;      /* 1 = Spiel laeuft, 0 = beenden */
    char status[80];
    int wx, wy;
    int nx, ny;

    welt_laden();                                       
    printf("[%c] [%c]\n", welt[9][0], welt[0][0]);

    if (tui_init() != 0) {
        fprintf(stderr, "Fehler beim Initialisieren des TUI\n");
        return 1;
    }

    while (laeuft) {

        /* 1. Eingabe: alle anliegenden Tasten lesen */
        nx=x;
        ny=y;

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
       if (frei(nx, (ny-1))==1){
        x = nx;
        y = ny;
       }

        /* 3. Zeichnen */
        tui_clear();
        
        for (wy = 0; wy < H; wy++) {
            for (wx = 0; wx < W; wx++) {
             char ch = welt[wy][wx];
               if (ch == '#') {
                tui_put(wx, wy+1, ch, TUI_COLOR_GREEN);
               }
               if (ch== '~'){
                tui_put(wx, wy+1,ch, TUI_COLOR_CYAN);
               }
            }
        }
        snprintf(status, sizeof(status), "x=%d y=%d   (q = beenden)", x, y);
        tui_text(0, 0, status, TUI_COLOR_WHITE);
        tui_text(x, y, SUB_SPRITE, TUI_COLOR_YELLOW);
        tui_present();

        /* 4. Warten */
        tui_sleep_ms(1000 / FPS);
    }

    tui_shutdown;
    return 0;

}