#include "welt.h"

#include <stdio.h>
#include <string.h>

int welt_laden(welt *w, const char *pfad)
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

char welt_at(const welt *w, int x, int y)
{
    if (y < 0) return '~';                                    /* oberhalb: Oberfläche */
    if (x < 0 || x >= w->breite || y >= w->hoehe) return '#';  /* sonst ringsum Fels */
    return w->zellen[y][x];
}

int welt_setzen(welt *w, int x, int y, char c)
{
    if (x < 0 || x >= w->breite || y < 0 || y >= w->hoehe) return -1;
    w->zellen[y][x] = c;
    return 0;
}
