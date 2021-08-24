

#also used by submakes
export ARCH=avr5
export ARCH_FAM=AVR
export DEV=atmega328p
export SCLOCK=20000000

#FLATLINE%ULIF$ Configs
export FLATLINE_ULIF=UART

DEFS =-D__FLATLINE_ULIF_AVR5_UART_BAUD=9600
DEFS +=-D__FLATLINE_ULIF_AVR5_UART_DATA=8
DEFS +=-D__FLATLINE_ULIF_AVR5_UART_PAST=0x01

#u/ limit (SID limit = 4000% + ULIMIT)
export ULIMIT=0x1000

export CC=avr-gcc
export AR=avr-ar
export OBJCOPY=avr-objcopy
export OBJDUMP=avr-objdump
export AVRSIZE=avr-size

export OFFSET_TEXT=0x000000
export OFFSET_DATA=0x800100

ATRI8=./xtools/atri8/atri8

#XERIS config
DEFS +=-D__XERIS_BUILD_ARCH_$(ARCH) -D__XERIS_BUILD_DEV_$(DEV)
DEFS +=-D__XERIS_BUILD_ARCH_FAM_$(ARCH_FAM)
DEFS +=-D__XERIS_BUILD_SFSYS=$(SCLOCK)
DEFS +=-D__XERIS_BUILD_ULIMIT=$(ULIMIT)

export DEFS

#XERIS Core Include Directory
XERIS_INCLUDE = $(realpath ./include)
export XERIS_INCLUDE

#flags for cc and ld
COMFLAGS = -nostartfiles -nostdlib -mrelax -O1

#need this for the built-ins gcc uses for maths, etc
LDLIBS += -lgcc

## Compilation options, type man avr-gcc if you're curious.
CPPFLAGS = -I$(abspath ./include)

#defines
CPPFLAGS += $(DEFS)

#Compiler Options
CFLAGS = -g -std=gnu99 -Wall -Wno-main
## Keep memory as tight as possible
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
## try to use rjmp and rcall as much as possible
CFLAGS += -fno-jump-tables
CFLAGS += $(COMFLAGS)

LDLIBS = -lgcc

LDFLAGS = -Wl,-Map,$@.map
## we can handle the start-up ourselves..
LDFLAGS += -mrelax
LDFLAGS += -Wl,--gc-sections,-q
LDFLAGS += $(COMFLAGS)


##
##LDFLAGS += -Wl,-Tdata,$(OFFSET_DATA)
##LDFLAGS += -Wl,-Ttext,$(OFFSET_TEXT)

TARGET_ARCH = -mmcu=$(ARCH)

#used in other Makes
export LDLIBS
export LDFLAGS
export TARGET_ARCH
export CPPFLAGS
export CFLAGS

## Core Packages
XERIS_CORE=opsec.elf flatline.elf atria.elf siplex.elf strap.elf

## Explicit pattern rules:
##	To make .o files from .c files
#%.a:
#	$(MAKE) -C $(dir $@) $(notdir $@)


flatline.elf:
	$(MAKE) -C ./FLATLINE $(notdir $@)
	mv ./FLATLINE/flatline.elf* ./

opsec.elf:
	$(MAKE) -C ./OPSEC $(notdir $@)
	mv ./OPSEC/opsec.elf* ./

atria.elf:
	$(MAKE) -C ./ATRIA $(notdir $@)
	mv ./ATRIA/atria.elf* ./

siplex.elf:
	$(MAKE) -C ./SIPLEX/STARLINE/AVR5 $(notdir $@)
	mv ./SIPLEX/STARLINE/AVR5/siplex.elf* ./

strap.elf:
	$(MAKE) -C ./STRAP $(notdir $@)
	mv ./STRAP/strap.elf* ./

timer.elf:
	$(MAKE) -C ./u/timer $(notdir $@)
	mv ./u/timer/timer.elf* ./

s_clr.elf:
	$(MAKE) -C ./s/clr $(notdir $@)
	mv ./s/clr/s_clr.elf* ./

s_rsv.elf:
	$(MAKE) -C ./s/rsv $(notdir $@)
	mv ./s/rsv/s_rsv.elf* ./

s_isu.elf:
	$(MAKE) -C ./s/isu $(notdir $@)
	mv ./s/isu/s_isu.elf* ./

s_txc.elf:
	$(MAKE) -C ./s/txc $(notdir $@)
	mv ./s/txc/s_txc.elf* ./

f_rdc.elf:
	$(MAKE) -C ./f/rdc $(notdir $@)
	mv ./f/rdc/f_rdc.elf* ./

f_man.elf:
	$(MAKE) -C ./f/man $(notdir $@)
	mv ./f/man/f_man.elf* ./

l_track.elf:
	$(MAKE) -C ./l/track $(notdir $@)
	mv ./l/track/l_track.elf* ./

u0.elf:
	$(MAKE) -C ./u $(notdir $@)
	mv ./u/u0.elf* ./

spi.elf:
	$(MAKE) -C ./u/spi $(notdir $@)
	mv ./u/spi/spi.elf* ./

#$(CC) $(LDFLAGS) $(TARGET_ARCH) $< $(LDLIBPATHS) $(LDLIBS) -lgcc -o $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
	$(OBJDUMP) -S $< > $@.lst

#%.eeprom: $(TARGET).elf
#	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@
#

%.ss: %.elf
	$(OBJDUMP) -D $< > $@

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

## These targets don't have files named after them
.PHONY: compile all disassemble disasm clean pack

## Full Builds ##

pack: $(XERIS_CORE) s_clr.elf s_rsv.elf s_isu.elf s_txc.elf \
f_rdc.elf timer.elf u0.elf spi.elf
	$(ATRI8) opsec.elf -a xeris.img 0000 opsec_cdd ffff ffff ffff
	$(ATRI8) flatline.elf -a xeris.img 0001 flatline ffff ffff ffff
	$(ATRI8) atria.elf -a xeris.img 0002 main ffff ffff ffff
	$(ATRI8) siplex.elf -a xeris.img 0003 siplex ffff ffff ffff
	$(ATRI8) strap.elf -a xeris.img 0004 main ffff ffff ffff
	$(ATRI8) s_clr.elf -a xeris.img 0011 main ffff ffff ffff
	$(ATRI8) s_rsv.elf -a xeris.img 0012 main ffff ffff ffff
	$(ATRI8) s_isu.elf -a xeris.img 0013 main ffff ffff ffff
	$(ATRI8) s_txc.elf -a xeris.img 0014 main ffff ffff ffff
	$(ATRI8) f_rdc.elf -a xeris.img 1005 main ffff ffff ffff -t 2
#	$(ATRI8) f_man.elf -a xeris.img 1234 main ffff ffff ffff
	$(ATRI8) timer.elf -a xeris.img 0005 main ffff ffff ffff -i clock_isr 16
	$(ATRI8) u0.elf -a xeris.img 1235 main ffff ffff ffff
	$(ATRI8) spi.elf -a xeris.img 1236 main ffff ffff ffff


##

# Optionally create listing file from .elf
# This creates approximate assembly-language equivalent of your code.
# Useful for debugging time-sensitive bits,
# or making sure the compiler does what you want.
disassemble: $(TARGET).ss $(TARGET).lst

disasm: disassemble

# Optionally show how big the resulting program is
size:  $(TARGET).elf
	$(AVRSIZE) -C --mcu=$(MCU) $(TARGET).elf

clean:
	find ./ -name "*.elf" -exec rm {} \;
	find ./ -name "*.o" -exec rm {} \;
	find ./ -name "*.map" -exec rm {} \;
	find ./ -name "*.lst" -exec rm {} \;
	find ./ -name "*~" -exec rm {} \;

sweep:
	rm -f *.elf *.hex *.obj *.o *.d *.eep *.lst *.lss *.sym *.map *~ *.eeprom
