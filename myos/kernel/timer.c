/* ============================================================
 *  MyOS - Driver Timer (PIT - Programmable Interval Timer)
 *  IRQ0, fréquence configurable
 * ============================================================ */

#include "../include/timer.h"
#include "../include/idt.h"

#define PIT_CHANNEL0    0x40
#define PIT_CMD         0x43
#define PIT_BASE_FREQ   1193180

static volatile uint32_t ticks = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* ---- Handler du timer ---- */
static void timer_handler(struct regs* r) {
    (void)r;
    ticks++;
}

/* ---- Récupérer le nombre de ticks ---- */
uint32_t timer_get_ticks(void) {
    return ticks;
}

/* ---- Attendre N ticks ---- */
void timer_wait(uint32_t n) {
    uint32_t end = ticks + n;
    while (ticks < end) {
        __asm__ volatile ("hlt");
    }
}

/* ---- Initialiser le timer ---- */
void timer_init(uint32_t frequency) {
    uint32_t divisor = PIT_BASE_FREQ / frequency;

    /* Mode 3 (square wave), channel 0 */
    outb(PIT_CMD, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)(divisor >> 8));

    irq_install_handler(0, timer_handler);
}
