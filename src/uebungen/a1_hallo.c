#include <stdio.h>

#include "tui.h"


int main(int argc, char argv[]){


    if(tui_init() !=0){
        fprintf(stderr, "Fehler beim initalisieren des TUI\n");
        return 1;
    }

    //löschen des Bildschirmes
    tui_clear();

    tui_text(10, 10,"hallo welt", TUI_COLOR_GREEN);

    tui_present();

    //wait for quit -> mit taste q
    while (tui_poll_key() != 'q'){
        tui_clear();
        tui_text(10, 10,"hallo welt", TUI_COLOR_GREEN);
        tui_text(1 ,1, "wating...", TUI_COLOR_BLUE);
        tui_present();
    }

    tui_shutdown();

}
