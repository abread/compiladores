#---------------------------------------------------------------
#             CONFIGURE THESE VARIABLES IF NEEDED
#---------------------------------------------------------------

ROOT = ../build
RTS_LIB_DIR = $(ROOT)/usr/lib

MAKEOPTS += ROOT=$(ROOT)

COMPILER=../og
ASM=yasm

#---------------------------------------------------------------
#    PROBABLY, NO NEED TO CHANGE ANYTHING BEYOND THIS POINT
#---------------------------------------------------------------

COMPILER_FLAGS =
LDFLAGS  = -m elf_i386 -L$(RTS_LIB_DIR) -lrts

SOURCES     = $(wildcard *.og)
OBJECTS     = $(SOURCES:%.og=%.o)

SOURCE_FOLDERS = $(shell find . -type d -not -name '.')
EXEC_FOLDERS = $(SOURCE_FOLDERS:%-src=%)
EXEC_CLEAN = $(SOURCE_FOLDERS:%-src=%-clean)

EXECUTABLES = $(SOURCES:%.og=%)

#---------------------------------------------------------------
#                DO NOT CHANGE AFTER THIS LINE
#---------------------------------------------------------------

.PHONY: all
all: $(EXECUTABLES) $(EXEC_FOLDERS)

%.c: %.og
	$(COMPILER) $< -o $@

%.asm: %.og
	$(COMPILER) $< -o $@

%.o: %.asm
	$(ASM) -felf32 $< -o $@

%:%.o
	$(LD) $< -o $@ $(LDFLAGS)

%:%-src
	$(MAKE) -C $< $(MAKEOPTS)
	cp $</$@ $@

.PHONY: clean factorial-clean
clean: $(EXEC_CLEAN)
	$(RM) $(OBJECTS) $(EXECUTABLES) $(EXEC_FOLDERS) *.c *.asm

%-clean:
	$(MAKE) -C $< $(MAKEOPTS) clean

#---------------------------------------------------------------
#                           THE END
#---------------------------------------------------------------
