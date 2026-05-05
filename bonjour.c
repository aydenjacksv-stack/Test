#include <windows.h>
#include <stdio.h>
#include <conio.h>

/* Enable ANSI/VT100 escape codes (Windows 10+) */
static void enable_vt(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

#define CLEAR       "\033[2J\033[H"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define MOVE(x, y)  printf("\033[%d;%dH", (y), (x))

int main(void) {
    enable_vt();
    printf(CLEAR HIDE_CURSOR);

    /* Helicopter body (static) */
    MOVE(23, 6);  printf("|");
    MOVE(12, 7);  printf("___________|___________");
    MOVE(12, 8);  printf("| H E L I C O P T E R >");
    MOVE(12, 9);  printf("|_______________________|");
    MOVE(22, 10); printf("| |");
    MOVE(21, 11); printf("/   \\");

    MOVE(2, 14);  printf("Appuyez sur une touche pour quitter...");

    /* Main rotor: 19 chars wide, hub at index 9, centered on col 23 */
    const char *rotor[4] = {
        "---------O---------",
        "    \\\\\\\\\\O/////    ",
        "         O         ",
        "    /////O\\\\\\\\\\    ",
    };

    /* Tail rotor char (right of '>') */
    const char tail[4] = {'-', '\\', '|', '/'};

    int frame = 0;
    while (!_kbhit()) {
        MOVE(14, 5);
        printf("%s", rotor[frame]);
        MOVE(36, 8);
        printf("%c", tail[frame]);
        frame = (frame + 1) % 4;
        fflush(stdout);
        Sleep(80);
    }

    printf(SHOW_CURSOR);
    MOVE(1, 16);
    printf("Au revoir !\n");
    return 0;
}
