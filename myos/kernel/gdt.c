/* ============================================================
 *  MyOS - Global Descriptor Table (GDT)
 *  Définit les segments mémoire pour le mode protégé
 * ============================================================ */

#include "../include/gdt.h"

#define GDT_ENTRIES 5

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr   gp;

/* Charger le GDT (défini en assembleur) */
extern void gdt_flush(uint32_t);

/* ---- Configurer une entrée GDT ---- */
void gdt_set_gate(int num, uint32_t base, uint32_t limit,
                  uint8_t access, uint8_t gran)
{
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    gdt[num].access      = access;
}

/* ---- Initialiser le GDT ---- */
void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base  = (uint32_t)&gdt;

    /* Segment null (requis) */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Segment code kernel (ring 0) */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Segment data kernel (ring 0) */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Segment code user (ring 3) */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    /* Segment data user (ring 3) */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_flush((uint32_t)&gp);
}
