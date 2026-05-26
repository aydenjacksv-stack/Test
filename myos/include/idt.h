#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* Entrée IDT (8 octets) */
struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

/* Pointeur IDT */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Registres sauvegardés lors d'une interruption */
struct regs {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void irq_install_handler(int irq, void (*handler)(struct regs*));

/* Handlers d'exception (ISR) */
extern void isr0(void);  /* Division by Zero */
extern void isr1(void);  /* Debug */
extern void isr2(void);  /* NMI */
extern void isr3(void);  /* Breakpoint */
extern void isr4(void);  /* Overflow */
extern void isr5(void);  /* Bound Range Exceeded */
extern void isr6(void);  /* Invalid Opcode */
extern void isr7(void);  /* Device Not Available */
extern void isr8(void);  /* Double Fault */
extern void isr13(void); /* General Protection Fault */
extern void isr14(void); /* Page Fault */

/* Handlers d'IRQ */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

#endif
