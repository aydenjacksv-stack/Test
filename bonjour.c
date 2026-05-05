#include <windows.h>
#include <stdio.h>
#include <conio.h>

static void gotoxy(HANDLE h, int x, int y) {
    COORD c = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(h, c);
}

int main(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cci = {1, FALSE};
    SetConsoleCursorInfo(h, &cci);
    system("cls");

    /* Rotor hub at (cx, cy) */
    const int cx = 22;
    const int cy = 4;

    /* Static helicopter body */
    gotoxy(h, cx,      cy + 1); printf("|");
    gotoxy(h, cx - 11, cy + 2); printf("___________|___________");
    gotoxy(h, cx - 11, cy + 3); printf("| H E L I C O P T E R >");
    gotoxy(h, cx - 11, cy + 4); printf("|_______________________|");
    gotoxy(h, cx - 1,  cy + 5); printf("| |");
    gotoxy(h, cx - 2,  cy + 6); printf("/   \\");

    gotoxy(h, 2, cy + 9);
    printf("Appuyez sur une touche pour quitter...");

    /* 4-frame main rotor (19 chars wide, hub at index 9) */
    const char *rotor[4] = {
        "---------O---------",
        "    \\\\\\\\\\O/////    ",
        "         O         ",
        "    /////O\\\\\\\\\\    ",
    };

    /* Tail rotor (right of the nose '>') */
    const char tail[4] = {'-', '\\', '|', '/'};

    int frame = 0;
    while (!_kbhit()) {
        gotoxy(h, cx - 9, cy);
        printf("%s", rotor[frame]);
        gotoxy(h, cx + 12, cy + 3);
        printf("%c", tail[frame]);
        frame = (frame + 1) % 4;
        Sleep(80);
    }

    cci.bVisible = TRUE;
    SetConsoleCursorInfo(h, &cci);
    gotoxy(h, 0, cy + 11);
    printf("Au revoir !\n");
    return 0;
}
