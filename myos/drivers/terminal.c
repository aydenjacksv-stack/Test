/* ============================================================
 *  MyOS - Driver Terminal VGA
 *  Mode texte 80x25, 16 couleurs
 * ============================================================ */

#include "../include/terminal.h"
#include <stdint.h>
#include <stddef.h>

/* Dimensions du terminal VGA */
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  ((volatile uint16_t*)0xB8000)

/* Ports VGA pour le curseur */
#define VGA_CTRL_REG    0x3D4
#define VGA_DATA_REG    0x3D5

/* État du terminal */
static size_t terminal_row;
static size_t terminal_col;
static uint8_t terminal_color;
static volatile uint16_t* terminal_buffer;

/* ---- Fonctions I/O bas niveau ---- */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* ---- Création d'une entrée VGA ---- */
static inline uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

/* ---- Mise à jour du curseur hardware ---- */
static void update_cursor(void) {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_col;
    outb(VGA_CTRL_REG, 14);
    outb(VGA_DATA_REG, (uint8_t)(pos >> 8));
    outb(VGA_CTRL_REG, 15);
    outb(VGA_DATA_REG, (uint8_t)(pos & 0xFF));
}

/* ---- Créer couleur ---- */
uint8_t make_color(vga_color_t fg, vga_color_t bg) {
    return vga_entry_color(fg, bg);
}

/* ---- Initialisation du terminal ---- */
void terminal_init(void) {
    terminal_row    = 0;
    terminal_col    = 0;
    terminal_color  = vga_entry_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;
    terminal_clear();
}

/* ---- Effacer l'écran ---- */
void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] =
                vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_col = 0;
    update_cursor();
}

/* ---- Définir la couleur ---- */
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

/* ---- Scroll d'une ligne ---- */
void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[(y-1) * VGA_WIDTH + x] =
                terminal_buffer[y * VGA_WIDTH + x];
        }
    }
    /* Effacer la dernière ligne */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] =
            vga_entry(' ', terminal_color);
    }
    if (terminal_row > 0) terminal_row--;
}

/* ---- Écrire un caractère à la position actuelle ---- */
static void terminal_putchar_at(char c, uint8_t color, size_t x, size_t y) {
    terminal_buffer[y * VGA_WIDTH + x] = vga_entry(c, color);
}

/* ---- Écrire un caractère (avec gestion \n, \t, \b) ---- */
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
    } else if (c == '\r') {
        terminal_col = 0;
    } else if (c == '\t') {
        terminal_col = (terminal_col + 8) & ~7;
        if (terminal_col >= VGA_WIDTH) {
            terminal_col = 0;
            terminal_row++;
        }
    } else if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
            terminal_putchar_at(' ', terminal_color, terminal_col, terminal_row);
        }
    } else {
        terminal_putchar_at(c, terminal_color, terminal_col, terminal_row);
        terminal_col++;
        if (terminal_col >= VGA_WIDTH) {
            terminal_col = 0;
            terminal_row++;
        }
    }

    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }

    update_cursor();
}

/* ---- Nouvelle ligne ---- */
void terminal_newline(void) {
    terminal_putchar('\n');
}

/* ---- Écrire N caractères ---- */
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

/* ---- Écrire une chaîne (null-terminated) ---- */
void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

/* ---- Écrire un nombre en hexadécimal ---- */
void terminal_writehex(uint32_t value) {
    const char hex_chars[] = "0123456789ABCDEF";
    char buf[11] = "0x00000000";
    for (int i = 9; i >= 2; i--) {
        buf[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    terminal_writestring(buf);
}

/* ---- Écrire un nombre décimal ---- */
void terminal_writedec(uint32_t value) {
    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    char buf[12];
    int i = 0;
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        terminal_putchar(buf[j]);
    }
}

/* ---- Positionner le curseur ---- */
void terminal_set_cursor(size_t row, size_t col) {
    terminal_row = row;
    terminal_col = col;
    update_cursor();
}

/* ---- Écrire avec une couleur spécifique ---- */
void terminal_write_colored(const char* str, vga_color_t fg, vga_color_t bg) {
    uint8_t old_color = terminal_color;
    terminal_color = vga_entry_color(fg, bg);
    terminal_writestring(str);
    terminal_color = old_color;
}
