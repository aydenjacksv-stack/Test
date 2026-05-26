/* ============================================================
 *  MyOS - Gestion Mémoire (Heap basique)
 *  Allocateur mémoire simple avec liste chaînée de blocs
 * ============================================================ */

#include "../include/memory.h"
#include "../include/terminal.h"

/* Zone heap (après le kernel en mémoire) */
#define HEAP_START  0x100000    /* 1 MB */
#define HEAP_SIZE   (4 * 1024 * 1024)  /* 4 MB */

/* En-tête de bloc mémoire */
typedef struct mem_block {
    uint32_t            magic;      /* Signature : 0xDEADBEEF */
    size_t              size;
    int                 free;
    struct mem_block*   next;
    struct mem_block*   prev;
} mem_block_t;

#define BLOCK_MAGIC     0xDEADBEEF
#define BLOCK_HEADER    sizeof(mem_block_t)

static mem_block_t* heap_start_block = NULL;
static uint32_t total_memory = 0;
static uint32_t used_memory  = 0;

/* ---- Initialiser la mémoire ---- */
void memory_init(uint32_t mem_lower, uint32_t mem_upper) {
    total_memory = (mem_lower + mem_upper) * 1024;

    /* Initialiser le premier bloc (toute la heap) */
    heap_start_block = (mem_block_t*)HEAP_START;
    heap_start_block->magic = BLOCK_MAGIC;
    heap_start_block->size  = HEAP_SIZE - BLOCK_HEADER;
    heap_start_block->free  = 1;
    heap_start_block->next  = NULL;
    heap_start_block->prev  = NULL;
    used_memory = 0;
}

/* ---- Fusionner les blocs libres adjacents ---- */
static void merge_free_blocks(void) {
    mem_block_t* curr = heap_start_block;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += BLOCK_HEADER + curr->next->size;
            curr->next  = curr->next->next;
            if (curr->next) curr->next->prev = curr;
        } else {
            curr = curr->next;
        }
    }
}

/* ---- Allouer de la mémoire ---- */
void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    /* Aligner sur 4 octets */
    size = (size + 3) & ~3;

    mem_block_t* curr = heap_start_block;
    while (curr) {
        if (curr->free && curr->size >= size) {
            /* Diviser le bloc si assez grand */
            if (curr->size >= size + BLOCK_HEADER + 4) {
                mem_block_t* new_block = (mem_block_t*)((uint8_t*)curr + BLOCK_HEADER + size);
                new_block->magic = BLOCK_MAGIC;
                new_block->size  = curr->size - size - BLOCK_HEADER;
                new_block->free  = 1;
                new_block->next  = curr->next;
                new_block->prev  = curr;
                if (curr->next) curr->next->prev = new_block;
                curr->next = new_block;
                curr->size = size;
            }
            curr->free = 0;
            used_memory += curr->size;
            return (void*)((uint8_t*)curr + BLOCK_HEADER);
        }
        curr = curr->next;
    }
    return NULL;  /* Out of memory */
}

/* ---- Libérer de la mémoire ---- */
void kfree(void* ptr) {
    if (!ptr) return;

    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - BLOCK_HEADER);
    if (block->magic != BLOCK_MAGIC) return;  /* Corruption détectée */

    block->free = 1;
    used_memory -= block->size;
    merge_free_blocks();
}

/* ---- Afficher les infos mémoire ---- */
void memory_info(void) {
    terminal_write_colored("=== Informations Mémoire ===\n", COLOR_CYAN, COLOR_BLACK);
    terminal_writestring("  Total RAM     : ");
    terminal_writedec(total_memory / 1024 / 1024);
    terminal_writestring(" MB\n");
    terminal_writestring("  Heap utilisée : ");
    terminal_writedec(used_memory / 1024);
    terminal_writestring(" KB / ");
    terminal_writedec(HEAP_SIZE / 1024);
    terminal_writestring(" KB\n");
}

/* ============================================================
 * Fonctions utilitaires mémoire
 * ============================================================ */

void* memset(void* ptr, int value, size_t size) {
    uint8_t* p = (uint8_t*)ptr;
    while (size--) *p++ = (uint8_t)value;
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (size--) *d++ = *s++;
    return dest;
}

int memcmp(const void* a, const void* b, size_t size) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    while (size--) {
        if (*pa != *pb) return (int)*pa - (int)*pb;
        pa++; pb++;
    }
    return 0;
}
