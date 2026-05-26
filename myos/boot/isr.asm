; ============================================================
;  MyOS - Interrupt Service Routines (ISR) + IRQ
;  Gestion des interruptions et exceptions CPU
; ============================================================

bits 32
section .text

; Macro pour ISR sans code d'erreur
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0         ; Code d'erreur fictif
    push byte %1        ; Numéro d'interruption
    jmp isr_common_stub
%endmacro

; Macro pour ISR avec code d'erreur
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

; Macro pour IRQ
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

; Définition des ISR (exceptions CPU)
ISR_NOERRCODE 0   ; Division by Zero
ISR_NOERRCODE 1   ; Debug
ISR_NOERRCODE 2   ; NMI
ISR_NOERRCODE 3   ; Breakpoint
ISR_NOERRCODE 4   ; Overflow
ISR_NOERRCODE 5   ; Bound Range Exceeded
ISR_NOERRCODE 6   ; Invalid Opcode
ISR_NOERRCODE 7   ; Device Not Available
ISR_ERRCODE   8   ; Double Fault
ISR_NOERRCODE 9   ; Coprocessor Segment Overrun
ISR_ERRCODE   10  ; Invalid TSS
ISR_ERRCODE   11  ; Segment Not Present
ISR_ERRCODE   12  ; Stack-Segment Fault
ISR_ERRCODE   13  ; General Protection Fault
ISR_ERRCODE   14  ; Page Fault
ISR_NOERRCODE 15
ISR_NOERRCODE 16  ; x87 FPU Error
ISR_NOERRCODE 17  ; Alignment Check
ISR_NOERRCODE 18  ; Machine Check
ISR_NOERRCODE 19  ; SIMD Exception

; Définition des IRQ
IRQ 0, 32   ; Timer PIT
IRQ 1, 33   ; Clavier PS/2
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; Handler C pour les ISR
extern isr_handler

; Stub commun ISR
isr_common_stub:
    pusha                   ; Sauvegarder EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    mov ax, ds
    push eax                ; Sauvegarder DS
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call isr_handler        ; Appeler le handler C
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8              ; Nettoyer int_no et err_code
    sti
    iret

; Handler C pour les IRQ
extern irq_handler

; Stub commun IRQ
irq_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call irq_handler
    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    popa
    add esp, 8
    sti
    iret
