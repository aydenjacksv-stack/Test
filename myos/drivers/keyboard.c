/* ============================================================
 *  MyOS - Driver Clavier PS/2
 *  Décode les scancodes du clavier en caractères ASCII
 * ============================================================ */

#include "../include/keyboard.h"
#include "../include/idt.h"

#define KEYBOARD_PORT       0x60
#define KEYBOARD_STATUS     0x64

/* ---- Buffer circulaire ---- */
static char kb_buffer[KEYBOARD_BUFFER_SIZE];
static volatile int kb_read_pos  = 0;
static volatile int kb_write_pos = 0;

/* ---- Scancode set 1 → ASCII (layout QWERTY) ---- */
static const char scancode_map[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', /* 0-9 */
    '9', '0', '-', '=',  8,  '\t','q', 'w', 'e', 'r', /* 10-19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,  /* 20-29 */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 30-39 */
    '\'','`',  0,  '\\','z', 'x', 'c', 'v', 'b', 'n', /* 40-49 */
    'm', ',', '.', '/',  0,  '*',  0,  ' ',  0,        /* 50-58 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    /* 59-68 */
    0,   0,   0,   0,   0,   0,   '-',  0,   0,   0,   /* 69-78 */
    '+',  0,   0,   0,   0,   0,   0,   0,   0,   0,   /* 79-88 */
    0,   0,                                             /* 89-90 */
};

/* ---- Scancode set 1 → ASCII (avec SHIFT) ---- */
static const char scancode_map_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+',  8,  '\t','Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~',  0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M', '<', '>', '?',  0,  '*',  0,  ' ',  0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static int shift_pressed  = 0;
static int caps_lock      = 0;
static int ctrl_pressed   = 0;

/* ---- Port I/O ---- */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* ---- Ajouter au buffer ---- */
static void kb_buffer_push(char c) {
    int next = (kb_write_pos + 1) % KEYBOARD_BUFFER_SIZE;
    if (next != kb_read_pos) {  /* Buffer non plein */
        kb_buffer[kb_write_pos] = c;
        kb_write_pos = next;
    }
}

/* ---- Handler d'interruption clavier (IRQ1) ---- */
static void keyboard_handler(struct regs* r) {
    (void)r;
    uint8_t scancode = inb(KEYBOARD_PORT);

    /* Touche relâchée (bit 7 = 1) */
    if (scancode & 0x80) {
        scancode &= 0x7F;
        if (scancode == 0x2A || scancode == 0x36) shift_pressed = 0;
        if (scancode == 0x1D) ctrl_pressed = 0;
        return;
    }

    /* Touches spéciales */
    switch (scancode) {
        case 0x2A: case 0x36:   /* Shift */
            shift_pressed = 1;
            return;
        case 0x3A:               /* Caps Lock */
            caps_lock = !caps_lock;
            return;
        case 0x1D:               /* Ctrl */
            ctrl_pressed = 1;
            return;
        case 0x0E:               /* Backspace */
            kb_buffer_push('\b');
            return;
        default:
            break;
    }

    /* Obtenir le caractère ASCII */
    if (scancode < 128) {
        char c;
        if (shift_pressed) {
            c = scancode_map_shift[scancode];
        } else {
            c = scancode_map[scancode];
        }

        /* Appliquer Caps Lock sur les lettres */
        if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
        if (caps_lock && c >= 'A' && c <= 'Z') c += 32;

        /* Ctrl+C → caractère spécial */
        if (ctrl_pressed && (c == 'c' || c == 'C')) {
            kb_buffer_push(3);  /* ASCII ETX (Ctrl+C) */
            return;
        }

        if (c != 0) {
            kb_buffer_push(c);
        }
    }
}

/* ---- Lire un caractère (bloquant) ---- */
char keyboard_getchar(void) {
    while (kb_read_pos == kb_write_pos) {
        __asm__ volatile ("hlt");
    }
    char c = kb_buffer[kb_read_pos];
    kb_read_pos = (kb_read_pos + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

/* ---- Vérifier si un caractère est disponible ---- */
int keyboard_haschar(void) {
    return kb_read_pos != kb_write_pos;
}

/* ---- Initialiser le driver clavier ---- */
void keyboard_init(void) {
    /* Vider le buffer du contrôleur clavier */
    while (inb(KEYBOARD_STATUS) & 1) {
        inb(KEYBOARD_PORT);
    }
    irq_install_handler(1, keyboard_handler);
}
