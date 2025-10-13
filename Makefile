UNAME_M := $(shell uname -m)

ifeq ($(UNAME_M),aarch64)
PREFIX:=i686-linux-gnu-
BOOTIMG:=/usr/local/grub/lib/grub/i386-pc/boot.img
GRUBLOC:=/usr/local/grub/bin/
else
PREFIX:=
BOOTIMG:=/usr/lib/grub/i386-pc/boot.img
GRUBLOC :=
endif

CC := $(PREFIX)gcc
LD := $(PREFIX)ld
SIZE := $(PREFIX)size

CFLAGS := -ffreestanding -I src -mgeneral-regs-only -mno-mmx -m32 -march=i386 \
          -fno-pie -fno-stack-protector -g3 -Wall

ODIR = obj
SDIR = src

OBJS = \
        start.o \
        kernel_main.o \
        terminal.o \
        rprintf.o \
        interrupt.o \
        keyboard.o \
        scancodes.o \
        page.o

OBJ = $(patsubst %,$(ODIR)/%,$(OBJS))

# --- Build Rules ------------------------------------------------------------

all: kernel

obj:
	mkdir -p $(ODIR)

$(ODIR)/%.o: $(SDIR)/%.c | obj
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.s | obj
	$(CC) $(CFLAGS) -c -o $@ $<

kernel: $(OBJ)
	$(LD) -melf_i386 $(OBJ) -Tkernel.ld -o $@
	$(SIZE) $@

# --- Create bootable disk image --------------------------------------------

rootfs.img: kernel grub.cfg
	dd if=/dev/zero of=rootfs.img bs=1M count=32
	@if [ -x "$$(command -v grub-mkimage)" ]; then \
	    grub-mkimage -p "(hd0,msdos1)/boot" -o grub.img -O i386-pc normal biosdisk multiboot multiboot2 configfile fat exfat part_msdos; \
	else \
	    echo "Error: grub-mkimage not found. Install grub-pc-bin."; \
	    exit 1; \
	fi
	dd if=$(BOOTIMG) of=rootfs.img conv=notrunc
	dd if=grub.img of=rootfs.img bs=512 seek=1 conv=notrunc
	echo 'start=2048, type=83, bootable' | sfdisk rootfs.img
	mkfs.vfat --offset 2048 -F16 rootfs.img
	mcopy -i rootfs.img@@1M kernel ::/
	mmd -i rootfs.img@@1M boot
	mcopy -i rootfs.img@@1M grub.cfg ::/boot
	@echo " -- BUILD COMPLETED SUCCESSFULLY --"

# --- Run and Clean ----------------------------------------------------------

run: kernel
	qemu-system-i386 -hda rootfs.img

clean:
	rm -rf $(ODIR) kernel grub.img rootfs.img
