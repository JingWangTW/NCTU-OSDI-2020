CC			= aarch64-linux-gnu-gcc
LD 			= aarch64-linux-gnu-ld
OBJ_CPY 	= aarch64-linux-gnu-objcopy
OBJ_DUMP	= aarch64-linux-gnu-objdump
EMULATOR	= qemu-system-aarch64

CFLAGS 		= -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -Werror -Wextra
INCLUDE_FLG = -Iinclude -I. -Ilib

BUILD_DIR			= build
KERNEL_BUILD_DIR   	= $(BUILD_DIR)/kernel
USER_BUILD_DIR		= $(BUILD_DIR)/user
DEBUG_DIR   		= debug

KERNEL_LINKER 	= link.ld
USER_LINKER   	= user_linker.ld
KERNEL_ELF		= $(BUILD_DIR)/kernel8.elf
USER_ELF		= $(BUILD_DIR)/user.elf 
USER_IMAGE		= $(BUILD_DIR)/user.img
USER_BINARY		= $(BUILD_DIR)/user.bin 

USER_SRCS	= $(wildcard src/*.c) $(wildcard lib/*.c)
USER_ASMS   = $(wildcard src/*.S) $(wildcard lib/*.S)
USER_SRC_OBJS = $(patsubst %.c, $(USER_BUILD_DIR)/%.o, $(USER_SRCS))
USER_ASM_OBJS = $(patsubst %.S, $(USER_BUILD_DIR)/%.S.o, $(USER_ASMS))
USER_OBJS   = $(USER_SRC_OBJS) $(USER_ASM_OBJS)

KERNEL_SRCS = $(wildcard kernel/*.c) $(wildcard kernel/*/*.c) $(wildcard lib/*.c)
KERNEL_ASMS	= $(wildcard kernel/*.S) $(wildcard kernel/*/*.S) $(wildcard lib/*.S)
KERNEL_SRC_OBJS = $(patsubst %.c, $(KERNEL_BUILD_DIR)/%.o, $(KERNEL_SRCS))
KERNEL_ASM_OBJS = $(patsubst %.S, $(KERNEL_BUILD_DIR)/%.S.o, $(KERNEL_ASMS))
KERNEL_OBJS = $(KERNEL_SRC_OBJS) $(KERNEL_ASM_OBJS)

.PHONY: all clean run force generate_symbol_asm

all: kernel8.img
force: clean kernel8.img

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(DEBUG_DIR)

kernel8.img: $(KERNEL_ELF) generate_symbol_asm
	$(OBJ_CPY) -O binary $< $@

$(KERNEL_ELF) : $(KERNEL_OBJS) $(USER_BINARY)
	$(LD) -nostdlib -nostartfiles  -T $(KERNEL_LINKER) $^ -o $@

$(KERNEL_BUILD_DIR)/%.S.o: %.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE_FLG) -c $< -o $@

$(KERNEL_BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE_FLG) -c $< -o $@

$(USER_BUILD_DIR)/%.S.o: %.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -fno-zero-initialized-in-bss $(INCLUDE_FLG) -c $< -o $@

$(USER_BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -fno-zero-initialized-in-bss $(INCLUDE_FLG) -c $< -o $@

$(USER_BINARY): $(USER_OBJS)
	$(LD) -T $(USER_LINKER) -o $(USER_ELF) $^
	$(OBJ_CPY) $(USER_ELF) -O binary $(USER_IMAGE)
	$(LD) -r -b binary $(USER_IMAGE) -o $@

run: kernel8.img
	$(EMULATOR) -M raspi3 -kernel kernel8.img -serial stdio

generate_symbol_asm:
	@rm -rf $(DEBUG_DIR)
	@mkdir $(DEBUG_DIR)
	readelf --all $(KERNEL_ELF) > $(DEBUG_DIR)/kernel_symbol
	readelf --all $(USER_ELF) > $(DEBUG_DIR)/user_symbol
	$(OBJ_DUMP) -d $(KERNEL_ELF)  > $(DEBUG_DIR)/kernel_asm
	$(OBJ_DUMP) -d $(USER_ELF)  > $(DEBUG_DIR)/user_asm

debug: CFLAGS += -ggdb3 -Og

debug: clean kernel8.img generate_symbol_asm
	$(EMULATOR) -M raspi3 -kernel kernel8.img -serial stdio -s -S