/*
 * tui.h -- Minimale Text-User-Interface-Bibliothek in reinem C (C11, POSIX)
 *
 * Idee und Funktionsumfang angelehnt an cpp-tui (https://github.com/jonoton/cpp-tui,
 * MIT-Lizenz, C++17). Diese Bibliothek ist eine eigenstaendige, stark vereinfachte
 * Neuimplementierung fuer die Lehre in reinem C.
 *
 * Aufgaben der Bibliothek:
 *   - Terminal in den Roh-Modus schalten und wieder zuruecksetzen
 *   - einen Bildpuffer (2D-Array) anbieten, in den gezeichnet wird
 *   - den Bildpuffer mit einem einzigen write() ausgeben (kein Flackern)
 *   - Tastendruecke nicht blockierend einlesen
 *   - Zeitmessung fuer eine gleichmaessige Bildrate
 *
 * Laeuft unter Linux, WSL und macOS.
 */
#ifndef TUI_H
#define TUI_H

/* Maximale Groesse des Bildpuffers. Groessere Terminals werden abgeschnitten. */
#define TUI_MAX_W 200
#define TUI_MAX_H 60

/* Rueckgabewerte von tui_poll_key(). Druckbare Zeichen werden als ihr
 * ASCII-Code zurueckgegeben, Sondertasten ab 1000. */
typedef enum {
    TUI_KEY_NONE = 0,
    TUI_KEY_UP = 1000,
    TUI_KEY_DOWN,
    TUI_KEY_LEFT,
    TUI_KEY_RIGHT,
    TUI_KEY_ESC
} tui_key;

/* Farben (ANSI). TUI_COLOR_DEFAULT ist die normale Terminalfarbe. */
typedef enum {
    TUI_COLOR_DEFAULT = 0,
    TUI_COLOR_RED,
    TUI_COLOR_GREEN,
    TUI_COLOR_YELLOW,
    TUI_COLOR_BLUE,
    TUI_COLOR_MAGENTA,
    TUI_COLOR_CYAN,
    TUI_COLOR_WHITE
} tui_color;

/* Terminal vorbereiten. Gibt 0 bei Erfolg zurueck, -1 wenn die Ausgabe kein
 * Terminal ist (z.B. in einer Pipe). tui_shutdown() wird zusaetzlich ueber
 * atexit() registriert, damit das Terminal auch bei einem Abbruch aufraeumt. */
int tui_init(void);

/* Terminal zuruecksetzen: Cursor sichtbar, normaler Bildschirm, Roh-Modus aus. */
void tui_shutdown(void);

/* Aktuelle Groesse des Bildpuffers in Zeichen. */
int tui_width(void);
int tui_height(void);

/* Bildpuffer mit Leerzeichen fuellen. Am Anfang jedes Bildes aufrufen. */
void tui_clear(void);

/* Ein Zeichen in den Bildpuffer schreiben. Positionen ausserhalb werden
 * ignoriert, der Aufrufer muss also nicht selbst pruefen. */
void tui_put(int x, int y, char ch, tui_color color);

/* Eine Zeichenkette ab (x, y) nach rechts schreiben. */
void tui_text(int x, int y, const char *s, tui_color color);

/* Bildpuffer auf den Bildschirm schreiben (genau ein write()). */
void tui_present(void);

/* Eine Taste lesen, ohne zu blockieren. Liefert TUI_KEY_NONE, wenn gerade
 * keine Taste anliegt. */
int tui_poll_key(void);

/* Millisekunden seit einem festen Startpunkt (monoton, nicht die Uhrzeit). */
long tui_now_ms(void);

/* Wartet die angegebene Zeit. */
void tui_sleep_ms(int ms);

#endif /* TUI_H */