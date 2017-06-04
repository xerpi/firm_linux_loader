ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_tools

TARGET		= firm_linux_loader_arm9
SOURCES		= source source/fatfs
INCLUDES	= include include/fatfs ../common

SFILES	= $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.s))
CFILES	= $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))

OBJS	= $(SFILES:.s=.o) $(CFILES:.c=.o)
DEPS	= $(OBJS:.o=.d)
INCLUDE	= $(foreach dir, $(INCLUDES), -I$(CURDIR)/$(dir)) -Iinclude_common

ARCH	= -mcpu=arm946e-s -march=armv5te -mlittle-endian -mthumb-interwork
ASFLAGS	= $(ARCH) $(COMMON_ARCH) $(INCLUDE) -x assembler-with-cpp
CFLAGS	= -Wall -O0 -fno-builtin -nostartfiles $(ARCH) $(INCLUDE)

.PHONY: all clean copy

all: $(TARGET).elf

$(TARGET).elf: $(OBJS)
	$(CC) -T link.ld -Wl,--nmagic $^ -o $@
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
%.o: %.s
	$(CC) $(ASFLAGS) -MMD -MP -c $< -o $@

clean:
	@rm -f $(OBJS) $(TARGET).elf $(DEPS)

-include $(DEPS)
