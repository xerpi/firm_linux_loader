ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_tools

TARGET		= firm_linux_loader
ARM9_ELF	= arm9/firm_linux_loader_arm9.elf
ARM11_ELF	= arm11/firm_linux_loader_arm11.elf

.PHONY: all clean copy .FORCE
.FORCE:

all: $(TARGET).firm

$(TARGET).firm: $(ARM9_ELF) $(ARM11_ELF)
	firmtool build $@ -D $(ARM9_ELF) $(ARM11_ELF) -C NDMA XDMA -i

$(ARM9_ELF): .FORCE
	@$(MAKE) -C arm9

$(ARM11_ELF): .FORCE
	@$(MAKE) -C arm11

clean:
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm11 clean
	@rm -f $(TARGET).firm

copy: $(TARGET).firm
	cp $< $(SD3DS)/luma/payloads/down_$(TARGET).firm && sync
