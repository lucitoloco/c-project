/*
 * main.c -- Beispiel fuer den Aufbau einer C-Datei
 */
#include <stdio.h>
#include "konfig.h"

#define QUADRAT(x) ((x) * (x))

#ifdef _WIN32
#error "Dieses Programm braucht Linux, macOS oder WSL."
#endif

static int bilder_gesamt = 0;

int tiefe_berechnen(int start, int schritte);

int main(void)
{
    int tiefe = tiefe_berechnen(2, 3);

#ifdef DEBUG
    printf("[%s:%d] tiefe = %d\n", __FILE__, __LINE__, tiefe);
#endif

    printf("U-Boot %s in Tiefe %d, %d Bilder pro Sekunde\n",
           SUB_SPRITE, tiefe, FPS);
    printf("Flaeche: %d\n", QUADRAT(tiefe + 1));
    bilder_gesamt++;
    return 0;
}

int tiefe_berechnen(int start, int schritte)
{
    return start + schritte;
}