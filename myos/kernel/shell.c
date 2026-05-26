/* ============================================================
 *  MyOS - Shell Interactif
 *  Interpréteur de commandes du kernel
 * ============================================================ */

#include "../include/shell.h"
#include "../include/terminal.h"
#include "../include/keyboard.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../include/timer.h"

#define CMD_MAX_LEN     256
#define CMD_HISTORY     16
#define MAX_ARGS        16
#define VERSION         "1.0.0"
#define OS_NAME         "MyOS"

/* Historique des commandes */
static char history[CMD_HISTORY][CMD_MAX_LEN];
static int  history_count = 0;
static int  history_index = 0;

/* Système de fichiers virtuel (RAM) */
#define VFS_MAX_FILES   32
#define VFS_MAX_NAME    64
#define VFS_MAX_CONTENT 1024

typedef struct {
    char name[VFS_MAX_NAME];
    char content[VFS_MAX_CONTENT];
    int  used;
    int  is_dir;
} vfs_file_t;

static vfs_file_t vfs[VFS_MAX_FILES];
static char current_dir[VFS_MAX_NAME] = "/";

/* ============================================================
 * Fonctions utilitaires
 * ============================================================ */

static void print_prompt(void) {
    terminal_write_colored(OS_NAME, COLOR_LIGHT_CYAN, COLOR_BLACK);
    terminal_write_colored("@kernel", COLOR_GREEN, COLOR_BLACK);
    terminal_write_colored(":", COLOR_WHITE, COLOR_BLACK);
    terminal_write_colored(current_dir, COLOR_LIGHT_BLUE, COLOR_BLACK);
    terminal_write_colored("$ ", COLOR_YELLOW, COLOR_BLACK);
}

static void readline(char* buf, size_t max) {
    size_t pos = 0;
    buf[0] = 0;

    while (1) {
        char c = keyboard_getchar();

        if (c == '\n') {
            terminal_putchar('\n');
            buf[pos] = 0;
            break;
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
                buf[pos] = 0;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        } else if (c == 3) {  /* Ctrl+C */
            terminal_writestring("^C\n");
            buf[0] = 0;
            break;
        } else if (c >= 32 && pos < max - 1) {
            buf[pos++] = c;
            buf[pos]   = 0;
            terminal_putchar(c);
        }
    }
}

static int parse_args(char* cmd, char* args[], int max_args) {
    int argc = 0;
    char* token = strtok(cmd, " \t");
    while (token && argc < max_args) {
        args[argc++] = token;
        token = strtok(NULL, " \t");
    }
    return argc;
}

/* ============================================================
 * VFS (Virtual File System en RAM)
 * ============================================================ */

static void vfs_init(void) {
    for (int i = 0; i < VFS_MAX_FILES; i++) {
        vfs[i].used = 0;
    }
    /* Créer les répertoires de base */
    vfs[0].used = 1; vfs[0].is_dir = 1;
    strcpy(vfs[0].name, "/");

    vfs[1].used = 1; vfs[1].is_dir = 1;
    strcpy(vfs[1].name, "/home");

    vfs[2].used = 1; vfs[2].is_dir = 1;
    strcpy(vfs[2].name, "/etc");

    /* Créer quelques fichiers par défaut */
    vfs[3].used = 1; vfs[3].is_dir = 0;
    strcpy(vfs[3].name, "/etc/motd");
    strcpy(vfs[3].content, "Bienvenue sur MyOS v" VERSION "!\nUn OS custom écrit en C et Assembly.\nTapez 'help' pour voir les commandes.\n");

    vfs[4].used = 1; vfs[4].is_dir = 0;
    strcpy(vfs[4].name, "/home/readme.txt");
    strcpy(vfs[4].content, "Ceci est votre répertoire home.\nVous pouvez créer des fichiers avec: echo contenu > fichier\n");
}

static vfs_file_t* vfs_find(const char* name) {
    for (int i = 0; i < VFS_MAX_FILES; i++) {
        if (vfs[i].used && strcmp(vfs[i].name, name) == 0) {
            return &vfs[i];
        }
    }
    return NULL;
}

static vfs_file_t* vfs_create(const char* name, int is_dir) {
    for (int i = 0; i < VFS_MAX_FILES; i++) {
        if (!vfs[i].used) {
            vfs[i].used   = 1;
            vfs[i].is_dir = is_dir;
            strcpy(vfs[i].name, name);
            vfs[i].content[0] = 0;
            return &vfs[i];
        }
    }
    return NULL;
}

/* ============================================================
 * Commandes du shell
 * ============================================================ */

/* ---- help ---- */
static void cmd_help(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_write_colored("\n╔══════════════════════════════════════╗\n", COLOR_CYAN, COLOR_BLACK);
    terminal_write_colored("║        MyOS Shell - Commandes        ║\n", COLOR_CYAN, COLOR_BLACK);
    terminal_write_colored("╚══════════════════════════════════════╝\n", COLOR_CYAN, COLOR_BLACK);

    terminal_write_colored("\n  Système:\n", COLOR_YELLOW, COLOR_BLACK);
    terminal_writestring("    help        - Afficher cette aide\n");
    terminal_writestring("    version     - Version du système\n");
    terminal_writestring("    uname       - Informations système\n");
    terminal_writestring("    uptime      - Temps de fonctionnement\n");
    terminal_writestring("    meminfo     - Infos mémoire\n");
    terminal_writestring("    reboot      - Redémarrer\n");
    terminal_writestring("    halt        - Arrêter le système\n");
    terminal_writestring("    clear       - Effacer l'écran\n");

    terminal_write_colored("\n  Fichiers:\n", COLOR_YELLOW, COLOR_BLACK);
    terminal_writestring("    ls [dir]    - Lister les fichiers\n");
    terminal_writestring("    cd <dir>    - Changer répertoire\n");
    terminal_writestring("    pwd         - Répertoire courant\n");
    terminal_writestring("    cat <file>  - Afficher un fichier\n");
    terminal_writestring("    touch <f>   - Créer un fichier\n");
    terminal_writestring("    mkdir <d>   - Créer un répertoire\n");
    terminal_writestring("    echo <txt>  - Afficher du texte\n");
    terminal_writestring("    write <f>   - Écrire dans un fichier\n");
    terminal_writestring("    rm <file>   - Supprimer un fichier\n");

    terminal_write_colored("\n  Outils:\n", COLOR_YELLOW, COLOR_BLACK);
    terminal_writestring("    calc <expr> - Calculatrice (ex: calc 5+3)\n");
    terminal_writestring("    color       - Tester les couleurs\n");
    terminal_writestring("    history     - Historique des commandes\n");
    terminal_writestring("    snake       - Mini jeu Snake !\n");
    terminal_writestring("    banner      - Afficher le logo MyOS\n");
    terminal_writestring("    cpuinfo     - Info processeur\n");
    terminal_writestring("    date        - Afficher la date/heure\n");
    terminal_putchar('\n');
}

/* ---- clear ---- */
static void cmd_clear(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_clear();
}

/* ---- version ---- */
static void cmd_version(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_write_colored(OS_NAME " Version " VERSION "\n", COLOR_LIGHT_GREEN, COLOR_BLACK);
    terminal_writestring("Kernel: MyOS Kernel 1.0 (i686)\n");
    terminal_writestring("Build:  " __DATE__ " " __TIME__ "\n");
    terminal_writestring("Auteur: Claude Code\n");
}

/* ---- uname ---- */
static void cmd_uname(int argc, char* argv[]) {
    int all = (argc > 1 && strcmp(argv[1], "-a") == 0);
    terminal_writestring(OS_NAME);
    if (all) {
        terminal_writestring(" myos 1.0 #1 " __DATE__ " i686 GNU/MyOS");
    }
    terminal_putchar('\n');
}

/* ---- uptime ---- */
static void cmd_uptime(int argc, char* argv[]) {
    (void)argc; (void)argv;
    uint32_t ticks = timer_get_ticks();
    uint32_t seconds = ticks / 100;
    uint32_t minutes = seconds / 60;
    uint32_t hours   = minutes / 60;
    terminal_writestring("Uptime: ");
    terminal_writedec(hours);
    terminal_writestring("h ");
    terminal_writedec(minutes % 60);
    terminal_writestring("m ");
    terminal_writedec(seconds % 60);
    terminal_writestring("s (");
    terminal_writedec(ticks);
    terminal_writestring(" ticks)\n");
}

/* ---- meminfo ---- */
static void cmd_meminfo(int argc, char* argv[]) {
    (void)argc; (void)argv;
    memory_info();
}

/* ---- ls ---- */
static void cmd_ls(int argc, char* argv[]) {
    const char* dir = (argc > 1) ? argv[1] : current_dir;
    int found = 0;

    terminal_write_colored("Contenu de ", COLOR_CYAN, COLOR_BLACK);
    terminal_write_colored(dir, COLOR_WHITE, COLOR_BLACK);
    terminal_write_colored(":\n", COLOR_CYAN, COLOR_BLACK);

    for (int i = 0; i < VFS_MAX_FILES; i++) {
        if (!vfs[i].used) continue;
        /* Filtrer par répertoire parent */
        const char* name = vfs[i].name;
        /* Chercher les entrées dans ce répertoire */
        size_t dir_len = strlen(dir);
        if (strncmp(name, dir, dir_len) == 0) {
            const char* rest = name + dir_len;
            if (dir_len > 1 && *rest == '/') rest++;
            else if (dir_len == 1) { /* root */
                if (*rest == '/') rest++;
            }
            /* Pas de sous-répertoire dans le reste */
            if (strlen(rest) > 0 && !strchr(rest, '/')) {
                if (vfs[i].is_dir) {
                    terminal_write_colored("  [DIR]  ", COLOR_LIGHT_BLUE, COLOR_BLACK);
                    terminal_write_colored(rest, COLOR_LIGHT_BLUE, COLOR_BLACK);
                } else {
                    terminal_writestring("  [FIL]  ");
                    terminal_writestring(rest);
                }
                terminal_putchar('\n');
                found++;
            }
        }
    }
    if (!found) terminal_writestring("  (vide)\n");
}

/* ---- cd ---- */
static void cmd_cd(int argc, char* argv[]) {
    if (argc < 2 || strcmp(argv[1], "/") == 0 || strcmp(argv[1], "~") == 0) {
        strcpy(current_dir, "/");
        return;
    }
    if (strcmp(argv[1], "..") == 0) {
        char* last = current_dir + strlen(current_dir) - 1;
        while (last > current_dir && *last != '/') last--;
        if (last == current_dir) { current_dir[1] = 0; }
        else { *last = 0; }
        return;
    }

    char new_path[VFS_MAX_NAME];
    if (argv[1][0] == '/') {
        strcpy(new_path, argv[1]);
    } else {
        strcpy(new_path, current_dir);
        if (strcmp(current_dir, "/") != 0) strcat(new_path, "/");
        strcat(new_path, argv[1]);
    }

    vfs_file_t* f = vfs_find(new_path);
    if (f && f->is_dir) {
        strcpy(current_dir, new_path);
    } else {
        terminal_write_colored("cd: ", COLOR_RED, COLOR_BLACK);
        terminal_writestring(argv[1]);
        terminal_writestring(": répertoire introuvable\n");
    }
}

/* ---- pwd ---- */
static void cmd_pwd(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_writestring(current_dir);
    terminal_putchar('\n');
}

/* ---- cat ---- */
static void cmd_cat(int argc, char* argv[]) {
    if (argc < 2) {
        terminal_writestring("Usage: cat <fichier>\n");
        return;
    }
    char path[VFS_MAX_NAME];
    if (argv[1][0] == '/') strcpy(path, argv[1]);
    else {
        strcpy(path, current_dir);
        if (strcmp(current_dir, "/") != 0) strcat(path, "/");
        strcat(path, argv[1]);
    }

    vfs_file_t* f = vfs_find(path);
    if (!f || f->is_dir) {
        terminal_write_colored("cat: ", COLOR_RED, COLOR_BLACK);
        terminal_writestring(argv[1]);
        terminal_writestring(": fichier introuvable\n");
        return;
    }
    terminal_writestring(f->content);
}

/* ---- touch ---- */
static void cmd_touch(int argc, char* argv[]) {
    if (argc < 2) { terminal_writestring("Usage: touch <fichier>\n"); return; }
    char path[VFS_MAX_NAME];
    if (argv[1][0] == '/') strcpy(path, argv[1]);
    else {
        strcpy(path, current_dir);
        if (strcmp(current_dir, "/") != 0) strcat(path, "/");
        strcat(path, argv[1]);
    }
    if (vfs_find(path)) { return; }  /* Déjà existant */
    if (!vfs_create(path, 0)) {
        terminal_writestring("Erreur: système de fichiers plein\n");
    }
}

/* ---- mkdir ---- */
static void cmd_mkdir(int argc, char* argv[]) {
    if (argc < 2) { terminal_writestring("Usage: mkdir <dir>\n"); return; }
    char path[VFS_MAX_NAME];
    if (argv[1][0] == '/') strcpy(path, argv[1]);
    else {
        strcpy(path, current_dir);
        if (strcmp(current_dir, "/") != 0) strcat(path, "/");
        strcat(path, argv[1]);
    }
    if (!vfs_create(path, 1)) {
        terminal_writestring("Erreur: impossible de créer le répertoire\n");
    } else {
        terminal_writestring("Répertoire créé: ");
        terminal_writestring(path);
        terminal_putchar('\n');
    }
}

/* ---- echo ---- */
static void cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) terminal_putchar(' ');
        terminal_writestring(argv[i]);
    }
    terminal_putchar('\n');
}

/* ---- write ---- */
static void cmd_write(int argc, char* argv[]) {
    if (argc < 2) { terminal_writestring("Usage: write <fichier>\n"); return; }
    char path[VFS_MAX_NAME];
    if (argv[1][0] == '/') strcpy(path, argv[1]);
    else {
        strcpy(path, current_dir);
        if (strcmp(current_dir, "/") != 0) strcat(path, "/");
        strcat(path, argv[1]);
    }

    vfs_file_t* f = vfs_find(path);
    if (!f) f = vfs_create(path, 0);
    if (!f) { terminal_writestring("Erreur: impossible de créer le fichier\n"); return; }

    terminal_writestring("Entrez le contenu (ligne vide pour terminer):\n");
    char line[256];
    f->content[0] = 0;
    while (1) {
        terminal_write_colored("> ", COLOR_YELLOW, COLOR_BLACK);
        readline(line, 256);
        if (line[0] == 0) break;
        if (strlen(f->content) + strlen(line) + 2 < VFS_MAX_CONTENT) {
            strcat(f->content, line);
            strcat(f->content, "\n");
        }
    }
    terminal_writestring("Fichier sauvegardé.\n");
}

/* ---- rm ---- */
static void cmd_rm(int argc, char* argv[]) {
    if (argc < 2) { terminal_writestring("Usage: rm <fichier>\n"); return; }
    char path[VFS_MAX_NAME];
    if (argv[1][0] == '/') strcpy(path, argv[1]);
    else {
        strcpy(path, current_dir);
        if (strcmp(current_dir, "/") != 0) strcat(path, "/");
        strcat(path, argv[1]);
    }
    vfs_file_t* f = vfs_find(path);
    if (!f) {
        terminal_write_colored("rm: ", COLOR_RED, COLOR_BLACK);
        terminal_writestring(argv[1]);
        terminal_writestring(": fichier introuvable\n");
        return;
    }
    f->used = 0;
    terminal_writestring("Supprimé: ");
    terminal_writestring(path);
    terminal_putchar('\n');
}

/* ---- calc ---- */
static void cmd_calc(int argc, char* argv[]) {
    if (argc < 2) {
        terminal_writestring("Usage: calc <expression>\nExemples: calc 5+3  calc 10*4  calc 100-25  calc 20/4\n");
        return;
    }

    /* Parser l'expression simple */
    char expr[64];
    strcpy(expr, argv[1]);
    if (argc > 2) {
        for (int i = 2; i < argc; i++) strcat(expr, argv[i]);
    }

    char* p = expr;
    int a = 0, negative = 0;
    if (*p == '-') { negative = 1; p++; }
    while (*p >= '0' && *p <= '9') { a = a * 10 + (*p - '0'); p++; }
    if (negative) a = -a;

    char op = *p++;
    if (!op) { terminal_writedec(a); terminal_putchar('\n'); return; }

    int b = 0; negative = 0;
    if (*p == '-') { negative = 1; p++; }
    while (*p >= '0' && *p <= '9') { b = b * 10 + (*p - '0'); p++; }
    if (negative) b = -b;

    int result = 0;
    int valid = 1;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) { terminal_write_colored("Erreur: division par zéro!\n", COLOR_RED, COLOR_BLACK); return; }
            result = a / b;
            break;
        case '%':
            if (b == 0) { terminal_write_colored("Erreur: modulo par zéro!\n", COLOR_RED, COLOR_BLACK); return; }
            result = a % b;
            break;
        default: valid = 0;
    }

    if (valid) {
        terminal_write_colored(expr, COLOR_YELLOW, COLOR_BLACK);
        terminal_write_colored(" = ", COLOR_WHITE, COLOR_BLACK);
        if (result < 0) { terminal_putchar('-'); result = -result; }
        terminal_writedec((uint32_t)result);
        terminal_putchar('\n');
    }
}

/* ---- color ---- */
static void cmd_color(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_writestring("\nPalette de couleurs MyOS:\n");
    for (int bg = 0; bg < 8; bg++) {
        for (int fg = 0; fg < 16; fg++) {
            uint8_t col = make_color((vga_color_t)fg, (vga_color_t)bg);
            terminal_setcolor(col);
            terminal_writestring(" Ab ");
        }
        terminal_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
        terminal_putchar('\n');
    }
    terminal_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
    terminal_putchar('\n');
}

/* ---- history ---- */
static void cmd_history(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_writestring("Historique des commandes:\n");
    for (int i = 0; i < history_count; i++) {
        terminal_write_colored("  ", COLOR_DARK_GREY, COLOR_BLACK);
        terminal_writedec(i + 1);
        terminal_writestring("  ");
        terminal_writestring(history[i]);
        terminal_putchar('\n');
    }
}

/* ---- banner ---- */
static void cmd_banner(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_write_colored(
    "\n"
    "  ███╗   ███╗██╗   ██╗ ██████╗ ███████╗\n"
    "  ████╗ ████║╚██╗ ██╔╝██╔═══██╗██╔════╝\n"
    "  ██╔████╔██║ ╚████╔╝ ██║   ██║███████╗\n"
    "  ██║╚██╔╝██║  ╚██╔╝  ██║   ██║╚════██║\n"
    "  ██║ ╚═╝ ██║   ██║   ╚██████╔╝███████║\n"
    "  ╚═╝     ╚═╝   ╚═╝    ╚═════╝ ╚══════╝\n"
    "\n",
    COLOR_LIGHT_CYAN, COLOR_BLACK);
    terminal_write_colored("       My Own Operating System v" VERSION "\n\n",
        COLOR_YELLOW, COLOR_BLACK);
}

/* ---- cpuinfo ---- */
static void cmd_cpuinfo(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_write_colored("=== CPU Information ===\n", COLOR_CYAN, COLOR_BLACK);

    /* CPUID instruction */
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];

    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );

    /* Vendor string */
    *(uint32_t*)vendor     = ebx;
    *(uint32_t*)(vendor+4) = edx;
    *(uint32_t*)(vendor+8) = ecx;
    vendor[12] = 0;

    terminal_writestring("  Fabricant : ");
    terminal_writestring(vendor);
    terminal_putchar('\n');

    /* Features */
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );

    terminal_writestring("  Famille   : ");
    terminal_writedec((eax >> 8) & 0xF);
    terminal_writestring(", Modèle: ");
    terminal_writedec((eax >> 4) & 0xF);
    terminal_putchar('\n');

    terminal_writestring("  Features  : ");
    if (edx & (1 << 0))  terminal_writestring("FPU ");
    if (edx & (1 << 4))  terminal_writestring("TSC ");
    if (edx & (1 << 5))  terminal_writestring("MSR ");
    if (edx & (1 << 23)) terminal_writestring("MMX ");
    if (edx & (1 << 25)) terminal_writestring("SSE ");
    if (edx & (1 << 26)) terminal_writestring("SSE2 ");
    terminal_putchar('\n');
}

/* ---- date ---- */
static void cmd_date(int argc, char* argv[]) {
    (void)argc; (void)argv;
    uint32_t ticks = timer_get_ticks();
    uint32_t seconds = ticks / 100;

    terminal_writestring("Uptime: ");
    terminal_writedec(seconds / 3600);
    terminal_writestring(":");
    if ((seconds % 3600) / 60 < 10) terminal_putchar('0');
    terminal_writedec((seconds % 3600) / 60);
    terminal_writestring(":");
    if (seconds % 60 < 10) terminal_putchar('0');
    terminal_writedec(seconds % 60);
    terminal_writestring(" depuis le démarrage\n");
    terminal_writestring("Date système: " __DATE__ " " __TIME__ " (compilation)\n");
}

/* ---- snake (mini jeu) ---- */
static void cmd_snake(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_clear();
    terminal_write_colored("=== SNAKE GAME ===\n", COLOR_GREEN, COLOR_BLACK);
    terminal_writestring("(Version simplifiée - démo)\n");
    terminal_writestring("Contrôles: Z=haut, S=bas, Q=gauche, D=droite, X=quitter\n\n");

    /* Snake basique en mode texte */
    int snake_x = 10, snake_y = 5;
    int food_x = 20, food_y = 8;
    int score = 0;
    int running = 1;

    /* Dessiner le cadre */
    for (int x = 0; x < 40; x++) {
        terminal_set_cursor(3, x);
        terminal_write_colored("#", COLOR_WHITE, COLOR_BLACK);
        terminal_set_cursor(18, x);
        terminal_write_colored("#", COLOR_WHITE, COLOR_BLACK);
    }
    for (int y = 3; y <= 18; y++) {
        terminal_set_cursor(y, 0);
        terminal_write_colored("#", COLOR_WHITE, COLOR_BLACK);
        terminal_set_cursor(y, 39);
        terminal_write_colored("#", COLOR_WHITE, COLOR_BLACK);
    }

    while (running) {
        /* Nourriture */
        terminal_set_cursor(food_y, food_x);
        terminal_write_colored("*", COLOR_RED, COLOR_BLACK);

        /* Snake */
        terminal_set_cursor(snake_y, snake_x);
        terminal_write_colored("O", COLOR_GREEN, COLOR_BLACK);

        /* Score */
        terminal_set_cursor(1, 0);
        terminal_writestring("Score: ");
        terminal_writedec(score);
        terminal_writestring("   ");

        /* Attendre input */
        timer_wait(10);
        if (!keyboard_haschar()) continue;

        char c = keyboard_getchar();
        int new_x = snake_x, new_y = snake_y;

        if (c == 'z' || c == 'Z') new_y--;
        else if (c == 's' || c == 'S') new_y++;
        else if (c == 'q' || c == 'Q') new_x--;
        else if (c == 'd' || c == 'D') new_x++;
        else if (c == 'x' || c == 'X') { running = 0; break; }

        /* Collision murs */
        if (new_x <= 0 || new_x >= 39 || new_y <= 3 || new_y >= 18) {
            terminal_set_cursor(20, 0);
            terminal_write_colored("GAME OVER! Score: ", COLOR_RED, COLOR_BLACK);
            terminal_writedec(score);
            terminal_putchar('\n');
            running = 0;
            break;
        }

        /* Effacer ancienne position */
        terminal_set_cursor(snake_y, snake_x);
        terminal_writestring(" ");

        snake_x = new_x;
        snake_y = new_y;

        /* Manger la nourriture */
        if (snake_x == food_x && snake_y == food_y) {
            score += 10;
            food_x = 2 + (timer_get_ticks() % 35);
            food_y = 4 + (timer_get_ticks() % 13);
        }
    }

    timer_wait(200);
    terminal_clear();
}

/* ---- reboot ---- */
static void cmd_reboot(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_write_colored("Redémarrage...\n", COLOR_YELLOW, COLOR_BLACK);
    timer_wait(100);
    /* Pulse le contrôleur 8042 pour reset */
    __asm__ volatile (
        "mov $0xFE, %al\n"
        "outb %al, $0x64\n"
    );
    while(1) __asm__ volatile ("hlt");
}

/* ---- halt ---- */
static void cmd_halt(int argc, char* argv[]) {
    (void)argc; (void)argv;
    terminal_write_colored("Système arrêté. Vous pouvez éteindre votre ordinateur.\n",
        COLOR_YELLOW, COLOR_BLACK);
    __asm__ volatile ("cli; hlt");
}

/* ============================================================
 * Table de dispatch des commandes
 * ============================================================ */

typedef struct {
    const char* name;
    void (*func)(int, char**);
    const char* description;
} command_t;

static const command_t commands[] = {
    {"help",    cmd_help,    "Afficher l'aide"},
    {"clear",   cmd_clear,   "Effacer l'écran"},
    {"cls",     cmd_clear,   "Effacer l'écran"},
    {"version", cmd_version, "Version du système"},
    {"uname",   cmd_uname,   "Infos système"},
    {"uptime",  cmd_uptime,  "Temps de fonctionnement"},
    {"meminfo", cmd_meminfo, "Informations mémoire"},
    {"ls",      cmd_ls,      "Lister les fichiers"},
    {"dir",     cmd_ls,      "Lister les fichiers"},
    {"cd",      cmd_cd,      "Changer de répertoire"},
    {"pwd",     cmd_pwd,     "Répertoire courant"},
    {"cat",     cmd_cat,     "Afficher un fichier"},
    {"touch",   cmd_touch,   "Créer un fichier"},
    {"mkdir",   cmd_mkdir,   "Créer un répertoire"},
    {"echo",    cmd_echo,    "Afficher du texte"},
    {"write",   cmd_write,   "Écrire dans un fichier"},
    {"rm",      cmd_rm,      "Supprimer un fichier"},
    {"calc",    cmd_calc,    "Calculatrice"},
    {"color",   cmd_color,   "Palette de couleurs"},
    {"history", cmd_history, "Historique"},
    {"snake",   cmd_snake,   "Mini jeu Snake"},
    {"banner",  cmd_banner,  "Logo MyOS"},
    {"cpuinfo", cmd_cpuinfo, "Info processeur"},
    {"date",    cmd_date,    "Date et heure"},
    {"reboot",  cmd_reboot,  "Redémarrer"},
    {"halt",    cmd_halt,    "Arrêter le système"},
    {NULL, NULL, NULL}
};

/* ============================================================
 * Boucle principale du shell
 * ============================================================ */

void shell_init(void) {
    vfs_init();
}

void shell_run(void) {
    char input[CMD_MAX_LEN];
    char cmd_copy[CMD_MAX_LEN];
    char* args[MAX_ARGS];

    /* Afficher le MOTD */
    vfs_file_t* motd = vfs_find("/etc/motd");
    if (motd) terminal_writestring(motd->content);

    while (1) {
        print_prompt();
        readline(input, CMD_MAX_LEN);

        /* Ignorer les lignes vides */
        if (input[0] == 0) continue;

        /* Ajouter à l'historique */
        if (history_count < CMD_HISTORY) {
            strcpy(history[history_count++], input);
        } else {
            /* Décaler l'historique */
            for (int i = 0; i < CMD_HISTORY - 1; i++) {
                strcpy(history[i], history[i+1]);
            }
            strcpy(history[CMD_HISTORY - 1], input);
        }
        history_index = history_count;

        /* Parser les arguments */
        strcpy(cmd_copy, input);
        int argc = parse_args(cmd_copy, args, MAX_ARGS);
        if (argc == 0) continue;

        /* Chercher la commande */
        int found = 0;
        for (int i = 0; commands[i].name; i++) {
            if (strcmp(args[0], commands[i].name) == 0) {
                commands[i].func(argc, args);
                found = 1;
                break;
            }
        }

        if (!found) {
            terminal_write_colored(args[0], COLOR_RED, COLOR_BLACK);
            terminal_writestring(": commande introuvable. Tapez 'help'.\n");
        }
    }
}
