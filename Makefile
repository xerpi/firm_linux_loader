ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_tools

TARGET		= firm_linux_loader
ARM9_SOURCES	= source_arm9 source_arm9/fatfs
ARM9_INCLUDES	= include_arm9 include_arm9/fatfs
ARM11_SOURCES	= source_arm11
ARM11_INCLUDES	= include_arm11

ARM9_SFILES	= $(foreach dir, $(ARM9_SOURCES), $(wildcard $(dir)/*.s))
ARM9_CFILES	= $(foreach dir, $(ARM9_SOURCES), $(wildcard $(dir)/*.c))
ARM11_SFILES	= $(foreach dir, $(ARM11_SOURCES), $(wildcard $(dir)/*.s))
ARM11_CFILES	= $(foreach dir, $(ARM11_SOURCES), $(wildcard $(dir)/*.c))

ARM9_OBJS	= $(ARM9_SFILES:.s=.arm9.o) $(ARM9_CFILES:.c=.arm9.o)
ARM9_INCLUDE	= $(foreach dir, $(ARM9_INCLUDES), -I$(CURDIR)/$(dir)) -Iinclude_common
ARM11_OBJS	= $(ARM11_SFILES:.s=.arm11.o) $(ARM11_CFILES:.c=.arm11.o)
ARM11_INCLUDE	= $(foreach dir, $(ARM11_INCLUDES), -I$(CURDIR)/$(dir)) -Iinclude_common

COMMON_ARCH	= -mlittle-endian -mthumb-interwork
ARM9_ARCH	= -mcpu=arm946e-s -march=armv5te
ARM11_ARCH	= -mcpu=mpcore -march=armv6k

ARM9_ASFLAGS	= $(ARM9_ARCH) $(COMMON_ARCH) $(ARM9_INCLUDE) -x assembler-with-cpp
ARM11_ASFLAGS	= $(ARM11_ARCH) $(COMMON_ARCH) $(ARM11_INCLUDE) -x assembler-with-cpp

ARM9_CFLAGS	= -Wall -O0 -fno-builtin -nostartfiles $(ARM9_ARCH) $(ARM9_INCLUDE)
ARM11_CFLAGS	= -Wall -O0 -fno-builtin -nostartfiles $(ARM11_ARCH) $(ARM11_INCLUDE)

.PHONY: all clean copy

all: $(TARGET).firm

$(TARGET).firm: $(TARGET).arm9.elf $(TARGET).arm11.elf
	firmtool build $@ \
		-n 0x08006800 -e 0x1FF80000 \
		-D $(TARGET).arm9.elf $(TARGET).arm11.elf \
		-A 0x08006800 0x1FF80000 \
		-C NDMA XDMA -i

$(TARGET).arm9.elf: $(ARM9_OBJS)
	$(CC) -T link_arm9.ld $^ -o $@

$(TARGET).arm11.elf: $(ARM11_OBJS)
	$(CC) -T link_arm11.ld $^ -o $@

%.arm9.o: %.c
	$(CC) $(ARM9_CFLAGS) -c $< -o $@
%.arm9.o: %.s
	$(CC) $(ARM9_ASFLAGS) -c $< -o $@

%.arm11.o: %.c
	$(CC) $(ARM11_CFLAGS) -c $< -o $@
%.arm11.o: %.s
	$(CC) $(ARM11_ASFLAGS) -c $< -o $@

clean:
	@rm -f $(ARM9_OBJS) $(ARM11_OBJS) $(TARGET).arm9.elf $(TARGET).arm11.elf $(TARGET).firm

copy: $(TARGET).firm
	cp $< $(SD3DS)/luma/payloads/down_$(TARGET).firm && sync
