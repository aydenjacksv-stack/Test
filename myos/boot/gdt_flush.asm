; ============================================================
;  MyOS - GDT Flush (Assembleur)
;  Charge le GDT et recharge les registres de segment
; ============================================================

bits 32
section .text

global gdt_flush

gdt_flush:
    mov eax, [esp+4]    ; Récupérer le pointeur GDT
    lgdt [eax]          ; Charger le GDT

    ; Recharger les segments en sautant au nouveau code segment (0x08)
    jmp 0x08:.reload_cs

.reload_cs:
    ; Recharger tous les registres de segment data (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
