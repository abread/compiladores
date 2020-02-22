# $Id: Makefile,v 1.1 2014/05/02 22:33:16 david Exp $
#---------------------------------------------------------------
#             CONFIGURE THESE VARIABLES IF NEEDED
#---------------------------------------------------------------

ROOT = ${HOME}/compiladores/root
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
EXECUTABLES = $(SOURCES:%.og=%)
TESTS       = $(EXECUTABLES:%=%-test)

#---------------------------------------------------------------
#                DO NOT CHANGE AFTER THIS LINE
#---------------------------------------------------------------

.PHONY:

all: $(EXECUTABLES) .PHONY

%.c: %.og $(COMPILER)
	$(COMPILER) $< -o $@

%.asm: %.og $(COMPILER)
	$(COMPILER) $< -o $@

%.o: %.asm
	$(ASM) -felf32 $< -o $@

%:%.o
	$(LD) $< -o $@ $(LDFLAGS)


test: .PHONY
	./testall.sh

# cool, but a bit ugly - let's not use them
test-mk: $(TESTS) .PHONY
%-test: %.out .PHONY
	diff --color $< expected/$<

%.out: %
	./$< > $@

clean: $(EXEC_CLEAN) .PHONY
	$(RM) $(OBJECTS) $(EXECUTABLES) *.c *.asm *.out

#---------------------------------------------------------------
#                           THE END
#---------------------------------------------------------------
