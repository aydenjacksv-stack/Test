/* ============================================================
 *  MyOS - Kernel Principal
 *  Point d'entrée C du kernel
 * ============================================================ */

#include "../include/terminal.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/memory.h"
#include "../include/keyboard.h"
#include "../include/timer.h"
#include "../include/shell.h"
#include <stdint.h>

/* Structure d'informations Multiboot */
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    /* ... autres champs ... */
};

/* ---- Affichage du splash screen de boot ---- */
static void print_boot_splash(void) {
    terminal_clear();

    /* Logo ASCII */
    terminal_write_colored(
        "  ___  ___      ___  ____  ____\n"
        " |   \\/   \\    /   ||    \\|    \\\n"
        " | .  .  . |  / .  ||  _  \\  _  \\\n"
        " | |\\/| |\\/| |/ /|  || | \\  | | \\  |\n"
        " |_|  |_|  |_/_/ |___||_|  \\_|_|  \\_|\n"
        "\n",
        COLOR_LIGHT_CYAN, COLOR_BLACK
    );

    terminal_write_colored(
        "         MyOS v1.0 - Custom Kernel\n"
        "    Démarrage en cours...\n\n",
        COLOR_YELLOW, COLOR_BLACK
    );
}

/* ---- Afficher la progression du boot ---- */
static void boot_step(const char* name, int ok) {
    terminal_writestring("  [ ");
    if (ok) terminal_write_colored(" OK ", COLOR_LIGHT_GREEN, COLOR_BLACK);
    else    terminal_write_colored("FAIL", COLOR_RED,         COLOR_BLACK);
    terminal_writestring(" ] ");
    terminal_writestring(name);
    terminal_putchar('\n');
}

/* ---- Point d'entrée principal du kernel ---- */
void kernel_main(uint32_t multiboot_magic, struct multiboot_info* mbi) {
    /* Initialiser le terminal en premier */
    terminal_init();
    print_boot_splash();

    /* Démarrage du kernel */
    terminal_write_colored("Initialisation du kernel...\n", COLOR_LIGHT_GREY, COLOR_BLACK);

    /* 1. GDT - Segmentation */
    gdt_init();
    boot_step("GDT (Segmentation mémoire)", 1);

    /* 2. IDT - Interruptions */
    idt_init();
    boot_step("IDT (Descripteurs d'interruption)", 1);

    /* 3. Mémoire */
    uint32_t mem_lower = 0, mem_upper = 0;
    if (multiboot_magic == 0x2BADB002 && mbi) {
        mem_lower = mbi->mem_lower;
        mem_upper = mbi->mem_upper;
    } else {
        mem_lower = 640;        /* 640 KB basse mémoire */
        mem_upper = 65536;      /* 64 MB haute mémoire */
    }
    memory_init(mem_lower, mem_upper);
    boot_step("Mémoire (heap allocateur)", 1);

    /* 4. Timer PIT */
    timer_init(100);    /* 100 Hz = 100 ticks/seconde */
    boot_step("Timer PIT (100 Hz)", 1);

    /* 5. Clavier */
    keyboard_init();
    boot_step("Clavier PS/2", 1);

    /* 6. Activer les interruptions */
    __asm__ volatile ("sti");
    boot_step("Interruptions activées", 1);

    /* 7. Shell */
    shell_init();
    boot_step("Shell interactif", 1);

    /* Afficher les infos mémoire */
    terminal_putchar('\n');
    terminal_write_colored("  RAM disponible: ~", COLOR_GREEN, COLOR_BLACK);
    terminal_writedec(mem_upper / 1024);
    terminal_write_colored(" MB\n", COLOR_GREEN, COLOR_BLACK);

    terminal_putchar('\n');
    terminal_write_colored("═══════════════════════════════════════════════════\n",
        COLOR_DARK_GREY, COLOR_BLACK);
    terminal_write_colored(" MyOS v1.0 prêt!  Tapez 'help' pour commencer.\n",
        COLOR_WHITE, COLOR_BLACK);
    terminal_write_colored("═══════════════════════════════════════════════════\n\n",
        COLOR_DARK_GREY, COLOR_BLACK);

    /* Lancer le shell */
    shell_run();

    /* Ne devrait jamais arriver */
    for(;;) __asm__ volatile ("hlt");
}
