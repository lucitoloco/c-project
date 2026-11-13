#include "uboot.h"

#include <math.h>

#define SUB_LEN 3
#define SCHUB 4.0f        /* Schub bei Tastendruck */
#define DRAG 2.0f         /* Wasserwiderstand */
#define VX_MIN 2.0f
#define VX_MAX 16.0f

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static int zelle(float v)
{
    return (int)floorf(v); /* floorf statt (int)v: rundet auch bei negativen Werten korrekt ab */
}

uboot uboot_start(float x, float y, float vx, float vy)
{
    uboot u;
    u.x = x;
    u.y = y;
    u.vx = vx;
    u.vy = vy;
    return u;
}

void uboot_tauchen(uboot *u, int richtung)
{
    u->vy += (float)richtung * SCHUB;
}

void uboot_tempo(uboot *u, int richtung)
{
    if (richtung > 0) u->vx = MIN(VX_MAX, u->vx + 2.0f);
    if (richtung < 0) u->vx = MAX(VX_MIN, u->vx - 2.0f);
}

void uboot_bewegen(uboot *u, const welt *w, float dt)
{
    int i, an_oberflaeche;
    float ny;

    /* waagerecht: ungeprueft, Felskontakt erkennt der Aufrufer selbst */
    u->x += u->vx * dt;

    /* senkrecht: Wasserwiderstand, Wasseroberflaeche haelt auf */
    u->vy -= u->vy * DRAG * dt;
    ny = u->y + u->vy * dt;
    an_oberflaeche = 0;
    for (i = 0; i < SUB_LEN; i++) {
        if (welt_at(w, zelle(u->x) + i, zelle(ny)) == '~') an_oberflaeche = 1;
    }
    if (an_oberflaeche) u->vy = 0.0f;
    else u->y = ny;
}

int uboot_zelle_x(const uboot *u) { return zelle(u->x); }
int uboot_zelle_y(const uboot *u) { return zelle(u->y); }
