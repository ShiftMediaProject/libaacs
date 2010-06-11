CC ?= cc
CFLAGS ?= -std=c99 -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L -D_REENTRANT \
	-Wall -Wdisabled-optimization -Wpointer-arith -Wredundant-decls \
	-Wcast-qual -Wwrite-strings -Wtype-limits -Wundef -Wextra \
	-Winline -O0 -g

TOOLS = genconfigfile

.PHONY: all clean $(TOOLS)
all:	$(TOOLS)

clean:
	@rm -f $(TOOLS) *.o

genconfigfile: genconfigfile.c
	gcc -o $@ $?
