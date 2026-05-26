#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char* str);
char*  strcpy(char* dest, const char* src);
char*  strncpy(char* dest, const char* src, size_t n);
char*  strcat(char* dest, const char* src);
char*  strncat(char* dest, const char* src, size_t n);
int    strcmp(const char* a, const char* b);
int    strncmp(const char* a, const char* b, size_t n);
char*  strchr(const char* str, int c);
char*  strtok(char* str, const char* delim);
char*  itoa(int value, char* str, int base);
int    atoi(const char* str);
char*  strupr(char* str);
char*  strlwr(char* str);

#endif
