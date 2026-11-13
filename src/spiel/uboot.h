#ifndef UBOOT_H
#define UBOOT_H

#include "welt.h"

typedef struct {
    float x;    /* linke Zelle (Heck) */
    float y;
    float vx;   /* Zellen pro Sekunde, nach rechts */
    float vy;   /* Zellen pro Sekunde, positiv heisst nach unten */
} uboot;

uboot uboot_start(float x, float y, float vx, float vy);

/* richtung: -1 (hoch), 0, +1 (runter) */
void uboot_tauchen(uboot *u, int richtung);

/* richtung: -1 (langsamer), 0, +1 (schneller) */
void uboot_tempo(uboot *u, int richtung);

/* Bewegt u einen Zeitschritt dt weiter (Schub, Wasserwiderstand,
 * Wasseroberflaeche haelt auf). Prueft nicht auf Fels/Minen/Perlen -
 * das ist Aufgabe des Aufrufers (siehe welt_at). */
void uboot_bewegen(uboot *u, const welt *w, float dt);

int uboot_zelle_x(const uboot *u);
int uboot_zelle_y(const uboot *u);

#endif /* UBOOT_H */
