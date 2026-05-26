# ============================================================
#  MyOS - Makefile
#  Système de build du kernel
# ============================================================

# Outils de compilation
AS      = nasm
CC      = gcc
LD      = ld
OBJCOPY = objcopy

# Flags de compilation kernel (i686, pas de stdlib)
CFLAGS  = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
          -fno-stack-protector -fno-pic \
          -I./include

ASFLAGS = -f elf32

LDFLAGS = -m elf_i386 -T linker.ld

# Fichiers objets
OBJS = \
    boot/boot.o     \
    boot/gdt_flush.o \
    boot/isr.o      \
    kernel/kernel.o  \
    kernel/gdt.o     \
    kernel/idt.o     \
    kernel/memory.o  \
    kernel/timer.o   \
    kernel/shell.o   \
    drivers/terminal.o \
    drivers/keyboard.o \
    lib/string.o

# Cible principale
all: myos.kernel myos.iso

# Compiler les fichiers assembleur
%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

# Compiler les fichiers C
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Linker le kernel
myos.kernel: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo ""
	@echo "✓ Kernel compilé: myos.kernel"
	@ls -lh myos.kernel

# Créer l'ISO bootable
myos.iso: myos.kernel
	@echo ""
	@echo "Création de l'image ISO..."
	@mkdir -p iso/boot/grub
	@cp myos.kernel iso/boot/myos.kernel
	@cp grub/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso iso/ 2>/dev/null
	@echo ""
	@echo "✓ ISO créée: myos.iso"
	@ls -lh myos.iso

# Nettoyer
clean:
	@find . -name "*.o" -delete
	@rm -f myos.kernel myos.iso
	@rm -rf iso/
	@echo "Nettoyé."

# Tester avec QEMU (si disponible)
run: myos.iso
	qemu-system-i386 -cdrom myos.iso -m 128M

run-kernel: myos.kernel
	qemu-system-i386 -kernel myos.kernel -m 128M

.PHONY: all clean run run-kernel
