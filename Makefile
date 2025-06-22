#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

# Optionnel : si votre toolchain n'est pas dans le PATH
TOOLCHAIN_PATH ?=

#---------------------------------------------------------------------------------
# Compiler prefix
#---------------------------------------------------------------------------------
PREFIX := $(TOOLCHAIN_PATH)arm-none-eabi-

export CC := $(PREFIX)gcc
export CXX := $(PREFIX)g++
export AS := $(PREFIX)as
export AR := $(PREFIX)ar
export OBJCOPY := $(PREFIX)objcopy
export OBJDUMP := $(PREFIX)objdump

#---------------------------------------------------------------------------------
# Check toolchain
#---------------------------------------------------------------------------------
TOOLCHAIN_CHECK := $(shell command -v $(CC) 2>/dev/null)
ifeq ($(TOOLCHAIN_CHECK),)
$(error [ERROR] arm-none-eabi-gcc not found. Please install the ARM toolchain or set TOOLCHAIN_PATH properly.)
endif

#---------------------------------------------------------------------------------
# Project structure
#---------------------------------------------------------------------------------
TARGET := newcode
BUILD := build
SOURCES := $(shell find source -type d 2>/dev/null || echo .)
INCLUDES := $(shell find include -type d 2>/dev/null || echo .)

#---------------------------------------------------------------------------------
# Architecture & flags
#---------------------------------------------------------------------------------
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -mfpu=vfpv2

CFLAGS := -g -Wall -O2 -mword-relocations \
          -fomit-frame-pointer -ffunction-sections \
          $(ARCH) \
          $(INCLUDE) -DARM11

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS := -g $(ARCH)

LDFLAGS = -T $(LINKERSCRIPT) $(SYMBOLS_FLAG) -g $(ARCH) -nostartfiles -nodefaultlibs -nostdlib -Wl,-Map,$(notdir $*.map)
LIBS := -lm -lgcc

LIBDIRS :=

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export LINKERSCRIPT := $(CURDIR)/linker.x
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

ifeq ($(strip $(CPPFILES)),)
	export LD := $(CC)
else
	export LD := $(CXX)
endif

export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(LINKERSCRIPT) $(TARGET).elf $(TARGET).bin $(TARGET).sym

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------

# Valeur par défaut si non fournie
CODEADDR ?= 0x00626000

# Vérifier si symbols.ld existe et l'ajouter aux LDFLAGS
SYMBOLS_LD := $(CURDIR)/../symbols.ld
ifneq ($(wildcard $(SYMBOLS_LD)),)
    SYMBOLS_FLAG := -T $(SYMBOLS_LD)
    $(info Using symbols file: $(SYMBOLS_LD))
else
    SYMBOLS_FLAG :=
    $(info No symbols.ld file found, proceeding without it)
endif

# Informations sur le projet
GAME_NAME := InazumaG
LOADER_OFFSET := 0x00560cf0
LOADER_MAXSIZE := 0x00000310

$(info Game Name:           $(GAME_NAME))
$(info Loader Offset:       $(LOADER_OFFSET))
$(info Loader maximum Size: $(LOADER_MAXSIZE))
$(info New Code Offset:     $(CODEADDR))

define LINKERSCRIPT_TEMPLATE
OUTPUT_ARCH(arm)

SECTIONS
{
    . = $(CODEADDR);
    .text : {
        __text_start = . ;
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.data)
        *(.bss)
        *(COMMON)
        __text_end  = . ;
    }
}
endef
export LINKERSCRIPT_TEMPLATE

all: $(OUTPUT).bin $(OUTPUT).sym
	@echo Destination address: $(CODEADDR)

$(OUTPUT).bin : $(OUTPUT).elf
	$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)

$(OUTPUT).sym : $(OUTPUT).elf
	$(OBJDUMP) -t $< > $@
	@echo written the symbol table ... $(notdir $@)

%.elf: $(OFILES)
	@echo generating linker script
	echo "$$LINKERSCRIPT_TEMPLATE" > $(LINKERSCRIPT)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

%.o: %.s
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(ASFLAGS) -c $< -o $@

-include $(DEPSDIR)/*.d

endif
#---------------------------------------------------------------------------------