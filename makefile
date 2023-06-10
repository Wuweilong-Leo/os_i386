BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/
ASFLAGS = -f elf

CFLAGS = -m32 -Wall $(LIB) -c -fno-builtin -std=c99 

LDFALGS = -Ttext $(ENTRY_POINT) -e main  -m elf_i386 -Map $(BUILD_DIR)/kernel.map

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
	$(BUILD_DIR)/timer.o  $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o\
	$(BUILD_DIR)/debug.o


$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h lib/stdint.h kernel/init.h kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c kernel/init.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/interrupt.h lib/stdint.h kernel/global.h lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/stdint.h lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/print.o: lib/kernel/print.c lib/kernel/print.h lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@


$(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h lib/kernel/print.h lib/stdint.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@


$(BUILD_DIR)/kernel.o: kernel/kernel.s
	$(AS) $(ASFLAGS) $< -o $@


$(BUILD_DIR)/kernel.bin :$(OBJS)
	$(LD) $(LDFALGS) $^ -o $@


.PHONY: mk_dir hd clean all

mk_dir:
	if [[ ! -d $(BUILD_DIR) ]];then mkdir $(BUILD_DIR);fi

hd:
	dd if=$(BUILD_DIR)/kernel.bin \
	   of=/os_i386/bochs/hd60M.img\
	   bs=512 count=200 seek=9 conv=notrunc
	
clean:
	cd $(BUILD_DIR) && rm -f ./*

build: $(BUILD_DIR)/kernel.bin

all: mk_dir build hd



