/* ============================================================
 *  MyOS - Interrupt Descriptor Table (IDT)
 *  Gestion des interruptions et remappage du PIC
 * ============================================================ */

#include "../include/idt.h"
#include "../include/terminal.h"

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr   idtp;

extern void idt_load(uint32_t);

/* ---- Ports I/O ---- */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* ---- Configurer une entrée IDT ---- */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags | 0x60;
}

/* ---- Messages d'exception ---- */
static const char* exception_messages[] = {
    "Division by Zero",
    "Debug Exception",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Exception 15",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Exception"
};

/* ---- Handler d'exception (appelé depuis isr.asm) ---- */
void isr_handler(struct regs* r) {
    terminal_write_colored("\n[KERNEL PANIC] ", COLOR_WHITE, COLOR_RED);
    if (r->int_no < 20) {
        terminal_write_colored(exception_messages[r->int_no], COLOR_YELLOW, COLOR_BLACK);
    } else {
        terminal_writestring("Unknown Exception");
    }
    terminal_writestring(" (int=");
    terminal_writedec(r->int_no);
    terminal_writestring(", err=");
    terminal_writehex(r->err_code);
    terminal_writestring(")\n");
    terminal_write_colored("System Halted.\n", COLOR_RED, COLOR_BLACK);

    __asm__ volatile ("cli; hlt");
}

/* Handlers IRQ (tableau de fonctions) */
#define IRQ_MAX 16
static void (*irq_handlers[IRQ_MAX])(struct regs*) = {0};

void irq_install_handler(int irq, void (*handler)(struct regs*)) {
    irq_handlers[irq] = handler;
}

/* ---- Handler d'IRQ (appelé depuis isr.asm) ---- */
void irq_handler(struct regs* r) {
    /* EOI (End of Interrupt) */
    if (r->int_no >= 40) {
        outb(0xA0, 0x20);   /* PIC esclave */
    }
    outb(0x20, 0x20);       /* PIC maître */

    /* Appeler le handler si installé */
    int irq_num = r->int_no - 32;
    if (irq_num >= 0 && irq_num < IRQ_MAX && irq_handlers[irq_num]) {
        irq_handlers[irq_num](r);
    }
}

/* ---- Remapper le PIC (éviter conflits avec exceptions CPU) ---- */
static void pic_remap(void) {
    /* Sauvegarder les masques */
    uint8_t mask1 = inb(0x21);
    uint8_t mask2 = inb(0xA1);

    /* ICW1: Début initialisation */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    /* ICW2: Vecteurs d'interruption (IRQ 0-7 → INT 32-39, IRQ 8-15 → INT 40-47) */
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    /* ICW3: Cascade */
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    /* ICW4: Mode 8086 */
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    /* Restaurer les masques */
    outb(0x21, mask1);
    outb(0xA1, mask2);
}

/* ---- Initialiser le IDT ---- */
void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base  = (uint32_t)&idt;

    /* Effacer le IDT */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* Installer les ISR (exceptions) */
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);

    /* Remapper le PIC */
    pic_remap();

    /* Installer les IRQ */
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    /* Charger le IDT */
    __asm__ volatile ("lidt (%0)" : : "r"(&idtp));
}
