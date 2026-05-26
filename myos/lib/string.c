/* ============================================================
 *  MyOS - Bibliothèque String
 *  Implémentation des fonctions string standards
 * ============================================================ */

#include "../include/string.h"

/* ---- Longueur d'une chaîne ---- */
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

/* ---- Copier une chaîne ---- */
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

/* ---- Copier N caractères ---- */
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = 0;
    return dest;
}

/* ---- Concaténer ---- */
char* strcat(char* dest, const char* src) {
    char* d = dest + strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

/* ---- Concaténer N ---- */
char* strncat(char* dest, const char* src, size_t n) {
    char* d = dest + strlen(dest);
    size_t i = 0;
    while (i < n && src[i]) { *d++ = src[i++]; }
    *d = 0;
    return dest;
}

/* ---- Comparer ---- */
int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

/* ---- Comparer N ---- */
int strncmp(const char* a, const char* b, size_t n) {
    while (n-- && *a && (*a == *b)) { a++; b++; }
    if (n == (size_t)-1) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

/* ---- Chercher un caractère ---- */
char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) return (char*)str;
        str++;
    }
    return NULL;
}

/* ---- Tokeniser une chaîne ---- */
static char* strtok_ptr = NULL;

char* strtok(char* str, const char* delim) {
    if (str) strtok_ptr = str;
    if (!strtok_ptr) return NULL;

    /* Sauter les délimiteurs */
    while (*strtok_ptr && strchr(delim, *strtok_ptr)) strtok_ptr++;
    if (!*strtok_ptr) return NULL;

    char* token = strtok_ptr;
    while (*strtok_ptr && !strchr(delim, *strtok_ptr)) strtok_ptr++;

    if (*strtok_ptr) {
        *strtok_ptr = 0;
        strtok_ptr++;
    }

    return token;
}

/* ---- Convertir entier en chaîne ---- */
char* itoa(int value, char* str, int base) {
    char tmp[32];
    int i = 0;
    int negative = 0;

    if (value == 0) {
        str[0] = '0';
        str[1] = 0;
        return str;
    }

    if (value < 0 && base == 10) {
        negative = 1;
        value = -value;
    }

    while (value > 0) {
        int rem = value % base;
        tmp[i++] = (rem < 10) ? '0' + rem : 'a' + rem - 10;
        value /= base;
    }

    if (negative) tmp[i++] = '-';

    int j = 0;
    while (i > 0) str[j++] = tmp[--i];
    str[j] = 0;
    return str;
}

/* ---- Convertir chaîne en entier ---- */
int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    while (*str == ' ') str++;
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result * sign;
}

/* ---- Convertir en majuscules ---- */
char* strupr(char* str) {
    char* p = str;
    while (*p) {
        if (*p >= 'a' && *p <= 'z') *p -= 32;
        p++;
    }
    return str;
}

/* ---- Convertir en minuscules ---- */
char* strlwr(char* str) {
    char* p = str;
    while (*p) {
        if (*p >= 'A' && *p <= 'Z') *p += 32;
        p++;
    }
    return str;
}
