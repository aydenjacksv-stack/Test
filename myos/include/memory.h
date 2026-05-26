#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

/* Gestion mémoire basique */
void  memory_init(uint32_t mem_lower, uint32_t mem_upper);
void* kmalloc(size_t size);
void  kfree(void* ptr);
void  memory_info(void);

/* Fonctions utilitaires mémoire */
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
int   memcmp(const void* a, const void* b, size_t size);

#endif
