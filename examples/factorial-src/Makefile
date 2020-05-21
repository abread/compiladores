RTS_LIB_DIR = $(ROOT)/usr/lib

COMPILER = ../../og
COMPILER_FLAGS =

ASM = yasm

LDFLAGS = -m elf_i386 -L$(RTS_LIB_DIR) -lrts

SOURCES = $(wildcard *.og)
OBJECTS = $(SOURCES:%.og=%.o)
EXECUTABLE = factorial

.PHONY: all clean
all: $(EXECUTABLE)

clean:
	$(RM) -f $(OBJECTS) *.asm $(EXECUTABLE)

%.asm: %.og
	$(COMPILER) $< -o $@

%.o: %.asm
	$(ASM) -felf32 $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(LD) $^ -o $@ $(LDFLAGS)
