.PHONY: all clean

SOURCES=globl.c statdat.c elf.c ui.c dump.c relo.c atri8.c pack.c sim.c

all: atri8

atri8: $(SOURCES)
	gcc -lcurses $(SOURCES) -o atri8

clean:
	rm atri8

