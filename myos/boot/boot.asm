; ============================================================
;  MyOS - Bootloader / Multiboot Entry Point
;  Architecture: x86 (i686)
;  Auteur: MyOS Project
; ============================================================

bits 32

; --- Constantes Multiboot ---
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_ALIGN     equ 1 << 0          ; Aligner les modules sur pages
MULTIBOOT_MEMINFO   equ 1 << 1          ; Infos mémoire
MULTIBOOT_FLAGS     equ MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; --- Section Multiboot Header (doit être dans les 8KB du début) ---
section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

; --- Stack du kernel (16KB) ---
section .bss
align 16
stack_bottom:
    resb 16384          ; 16 KB de stack
stack_top:

; --- Point d'entrée du kernel ---
section .text
global _start
extern kernel_main

_start:
    ; Initialiser le registre de pile
    mov esp, stack_top

    ; Sauvegarder les infos Multiboot
    push eax            ; Magic number multiboot
    push ebx            ; Adresse structure multiboot

    ; Appeler le kernel principal (C)
    call kernel_main

    ; Si kernel_main retourne (ne devrait pas), halt
.hang:
    cli                 ; Désactiver les interruptions
    hlt                 ; Halter le processeur
    jmp .hang           ; Boucle infinie (sécurité)
