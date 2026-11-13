#include <stdio.h>
#include "welt.h"
#include "uboot.h"

static int pruefe(const char *name, int ok)
{
    printf("%-40s %s\n", name, ok ? "ok" : "FEHLER");
    return ok ? 0 : 1;
}

int main(void)
{
    welt w = {0};
    uboot u;
    float x_vorher, ny_oberflaeche_y;
    int fehler = 0;

    /* U-Boot faehrt von selbst nach rechts */
    u = uboot_start(1.0f, 4.5f, 8.0f, 0.0f);
    x_vorher = u.x;
    uboot_bewegen(&u, &w, 0.1f);
    fehler += pruefe("U-Boot faehrt nach rechts", u.x > x_vorher);

    /* Wasseroberflaeche haelt das U-Boot auf */
    u = uboot_start(1.0f, 1.0f, 0.0f, -10.0f);
    ny_oberflaeche_y = u.y;
    uboot_bewegen(&u, &w, 0.1f);
    fehler += pruefe("Oberflaeche haelt U-Boot", u.y <= ny_oberflaeche_y);

    /* uboot_tempo haelt sich an die Grenzen 2..16 */
    u = uboot_start(0.0f, 0.0f, 2.0f, 0.0f);
    uboot_tempo(&u, -1);
    fehler += pruefe("Tempo nicht unter 2", u.vx >= 2.0f);
    u = uboot_start(0.0f, 0.0f, 16.0f, 0.0f);
    uboot_tempo(&u, 1);
    fehler += pruefe("Tempo nicht ueber 16", u.vx <= 16.0f);

    /* welt_setzen aendert genau eine Zelle */
    {
        welt lv = {0};
        lv.breite = 3;
        lv.hoehe = 1;
        lv.zellen[0][0] = '*';
        welt_setzen(&lv, 0, 0, ' ');
        fehler += pruefe("welt_setzen loescht Zelle", welt_at(&lv, 0, 0) == ' ');
    }

    return fehler;
}
