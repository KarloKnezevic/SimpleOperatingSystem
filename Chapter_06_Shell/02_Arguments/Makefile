# Building system script (for 'make')

#------------------------------------------------------------------------------
# Common configuration that affects both kernel and user programs
#------------------------------------------------------------------------------

OS_NAME = "OSEER"
NAME_MAJOR := $(shell basename "`cd ..; pwd -P`")
NAME_MINOR := $(shell basename "`pwd -P`")
PROJECT := $(NAME_MINOR)

PLATFORM = i386
VERSION = 1.0
AUTHOR = leonardo@zemris.fer.hr

# Intermediate and output files are placed into BUILDDIR
BUILDDIR = build

BINFILE = $(BUILDDIR)/$(PROJECT).elf
CDIMAGE = $(BUILDDIR)/$(PROJECT).iso

CMACROS = OS_NAME="\"$(OS_NAME)\"" PROJECT="\"$(PROJECT)\"" \
	  NAME_MAJOR="\"$(NAME_MAJOR)\"" NAME_MINOR="\"$(NAME_MINOR)\"" \
	  PLATFORM="\"$(PLATFORM)\"" AUTHOR="\"$(AUTHOR)\"" \
	  VERSION="\"$(VERSION)\""

CMACROS += DEBUG

CC = gcc
CFLAGS = -O3 -m32 -Wall -ffreestanding -nostdlib -fno-stack-protector \
	-fdata-sections -ffunction-sections

LINK = ld
LDSCRIPT = arch/$(PLATFORM)/ldscript.ld
LDFLAGS = -O3 -melf_i386 -T $(LDSCRIPT) --gc-sections -s


#------------------------------------------------------------------------------
# Devices

#"defines"
DEVICES = VGA_TEXT I8042 I8259 I8253 UART

#devices interface (variables implementing device_t interface)
DEVICES_DEV = vga_text_dev uart_com1 i8042_dev

comma := ,
empty :=
space := $(empty) $(empty)
DEV_VARS := $(subst $(space),$(comma),$(DEVICES_DEV))
DEV_PTRS := $(addprefix \&,$(DEVICES_DEV))
DEV_PTRS := $(subst $(space),$(comma),$(DEV_PTRS))

CMACROS += $(DEVICES) DEVICES_DEV=$(DEV_VARS) DEVICES_DEV_PTRS=$(DEV_PTRS) \
	IC_DEV=i8259 TIMER=i8253 K_INITIAL_STDOUT=vga_text_dev		   \
	K_STDOUT="\"COM1\"" U_STDOUT="\"VGA_TXT\"" U_STDIN="\"i8042\""
#	K_STDOUT="\"VGA_TXT\"" U_STDOUT="\"COM1\"" U_STDIN="\"COM1\""

#------------------------------------------------------------------------------
# Memory
# Memory allocators: 'gma' and/or 'first_fit'

FIRST_FIT = 1
GMA = 2

CMACROS += FIRST_FIT=$(FIRST_FIT) GMA=$(GMA)

MEM_ALLOCATOR = $(FIRST_FIT)
CMACROS += MEM_ALLOCATOR=$(MEM_ALLOCATOR)

#------------------------------------------------------------------------------
# Stack

CMACROS += STACK_SIZE=0x1000

#------------------------------------------------------------------------------
all: $(CDIMAGE)
#------------------------------------------------------------------------------

DIRS_K := arch/$(PLATFORM)/arch arch/$(PLATFORM)/arch/devices kernel devices
DIRS_P := programs/api programs/hello_world programs/timer programs/keyboard \
	programs/shell programs/arguments
LIBS = lib lib/mm

DIRS := $(DIRS_K) $(DIRS_P) $(LIBS)

INCLUDES := . programs arch/$(PLATFORM)

FILES := $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c $(DIR)/*.S))
OBJS := $(addprefix $(BUILDDIR)/,$(FILES:.c=.o))
OBJS := $(OBJS:.S=.asm.o)
DEPS := $(OBJS:.o=.d)

# dummy file that indicate directories for kernel objects are created
DIRS_CREATED = $(BUILDDIR)/.null

# create required directories in $(BUILDDIR) directory (including $(BUILDDIR))
$(DIRS_CREATED):
	@-if [ ! -e $(BUILDDIR) ]; then mkdir -p $(BUILDDIR); fi;
	@-$(foreach DIR,$(DIRS), if [ ! -e $(BUILDDIR)/$(DIR) ]; \
		then mkdir -p $(BUILDDIR)/$(DIR); fi; )
	@touch $(DIRS_CREATED)

# define how to compile .c files
$(BUILDDIR)/%.o: %.c $(DIRS_CREATED)
	@echo [compiling] $< ...
	@$(CC) -c $< -o $@ -MMD $(CFLAGS) \
		$(foreach INC,$(INCLUDES),-I $(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO))

# define how to compile .S files (assembler)
$(BUILDDIR)/%.asm.o: %.S $(DIRS_CREATED)
	@echo [compiling] $< ...
	@$(CC) -c $< -o $@ -MMD $(CFLAGS) \
		$(foreach INC,$(INCLUDES),-I$(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO))


# OS image
$(BINFILE): $(OBJS)
	@echo [linking $@]
	@$(LINK) $(LDFLAGS) $(OBJS) -o$@


BOOTCD := $(BUILDDIR)/cd
GRUBMENU := $(BOOTCD)/boot/grub/menu.lst
GRUBFILE := $(BOOTCD)/boot/grub/stage2_eltorito
GRUBFILE_ORIG := arch/$(PLATFORM)/grub_file

$(GRUBFILE):
	@-if [ ! -e $(BOOTCD) ]; then mkdir -p $(BOOTCD)/boot/grub ; fi;
	@cp -a $(GRUBFILE_ORIG) $(GRUBFILE)

$(GRUBMENU):
	@-if [ ! -e $(BOOTCD) ]; then mkdir -p $(BOOTCD)/boot/grub ; fi;
	@echo "default 0" > $(GRUBMENU)
	@echo "timeout=0" >> $(GRUBMENU)
	@echo "title $(PROJECT)" >> $(GRUBMENU)
	@echo "root (cd)" >> $(GRUBMENU)
	@echo "kernel /boot/$(PROJECT).elf" >> $(GRUBMENU)
	@echo "boot" >> $(GRUBMENU)

# ISO CD image for booting (with grub as boot loader and $(KERNEL) as OS image)
$(CDIMAGE): $(BINFILE) $(GRUBFILE) $(GRUBMENU)
	@cp $(BINFILE) $(BOOTCD)/boot/$(PROJECT).elf
	@mkisofs -m '.svn' -J -R -b boot/grub/stage2_eltorito		\
	-no-emul-boot -boot-load-size 4 -boot-info-table -V $(PROJECT)	\
	-A $(PROJECT) -o $(CDIMAGE) $(BOOTCD) 2> /dev/null
	@echo
	@echo ISO CD image: $(CDIMAGE)
	@echo

clean:
	@echo Cleaning.
	@-rm -f $(OBJS) $(DEPS) $(CDIMAGE) $(BINFILE)

clean_all cleanall:
	@echo Removing build directory!
	@-rm -rf $(BUILDDIR)

# starting compiled system in 'qemu' emulator
qemu: $(CDIMAGE)
	@echo Starting...
	@-qemu -no-kvm -cdrom $(CDIMAGE) -serial stdio

# DEBUGGING
# For debugging to work: include '-g' in CFLAGS and omit -s and -S from LDFLAGS
# Best if -O3 flag is also ommited from CFLAGS and LDFLAGS (or some variables
# may be optimized away)
# Start debugging from two consoles: 1st: make debug_qemu 2nd: make debug_gdb
debug_qemu: $(CDIMAGE)
	@echo Starting qemu ...
	@qemu -s -S -no-kvm -cdrom $(CDIMAGE) -serial stdio
debug_gdb: $(CDIMAGE)
	@echo Starting gdb ...
	@gdb -s $(BINFILE) -ex 'target remote localhost:1234'

-include $(DEPS)
