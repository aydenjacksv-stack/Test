#include <windows.h>
#include <stdio.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    printf("Bonjour !\n\nAppuyez sur Entree pour quitter...");
    getchar();
    return 0;
}
