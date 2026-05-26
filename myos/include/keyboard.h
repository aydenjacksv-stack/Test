#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
char keyboard_getchar(void);
int  keyboard_haschar(void);

/* Buffer circulaire clavier */
#define KEYBOARD_BUFFER_SIZE 256

#endif
