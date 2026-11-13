#ifndef WELT_H
#define WELT_H

#define W_MAX 400
#define H_MAX 40

typedef struct {
    char zellen[H_MAX][W_MAX];
    int breite;
    int hoehe;
} welt;

/* Liest die Level-Datei pfad in w. Rueckgabe: 0 bei Erfolg, -1 bei Fehler. */
int welt_laden(welt *w, const char *pfad);

/* Liefert das Zeichen an (x, y). Ausserhalb des Levels: '~' oberhalb,
 * sonst '#' (Fels als Begrenzung). */
char welt_at(const welt *w, int x, int y);

/* Setzt das Zeichen an (x, y). Rueckgabe: 0 bei Erfolg, -1 ausserhalb des Levels. */
int welt_setzen(welt *w, int x, int y, char c);

#endif /* WELT_H */
