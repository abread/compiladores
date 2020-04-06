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
EXECUTABLES = $(SOURCES:%.og=%)
TESTS       = $(EXECUTABLES:%=%-test)

#---------------------------------------------------------------
#                DO NOT CHANGE AFTER THIS LINE
#---------------------------------------------------------------

.PHONY:

all: $(EXECUTABLES) .PHONY

%.c: %.og $(COMPILER)
	$(COMPILER) $< -o $@ || (rm -f $@ && false)

%.asm: %.og $(COMPILER)
	$(COMPILER) $< -o $@ || (rm -f $@ && false)

%.xml: %.og $(COMPILER)
	$(COMPILER) $< --target xml -o $@ || (rm -f $@ && false)

%.o: %.asm
	$(ASM) -felf32 $< -o $@ || (rm -f $@ && false)

%:%.o
	$(LD) $< -o $@ $(LDFLAGS) || (rm -f $@ && false)


test: .PHONY
	./testall.sh

# cool, but a bit ugly - let's not use them
test-mk: $(TESTS) .PHONY
%-test: %.out .PHONY
	diff --color $< expected/$<

%.out: %
	./$< > $@

clean: $(EXEC_CLEAN) .PHONY
	$(RM) $(OBJECTS) $(EXECUTABLES) *.c *.asm *.out *.xml

#---------------------------------------------------------------
#                           THE END
#---------------------------------------------------------------
