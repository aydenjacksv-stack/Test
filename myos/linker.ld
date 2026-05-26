/* ============================================================
 *  MyOS - Script Linker
 *  Définit la structure de l'image kernel en mémoire
 * ============================================================ */

ENTRY(_start)

SECTIONS {
    /* Charger le kernel à 1 MB (0x100000) */
    . = 0x100000;

    /* Section texte (code exécutable) */
    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)   /* Header multiboot en premier */
        *(.text)
    }

    /* Section données initialisées */
    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    /* Section données en lecture seule */
    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    /* Section BSS (données non initialisées) */
    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }

    /* Fin du kernel */
    kernel_end = .;
}
