LEX ?= lex
LFLAGS ?=
YACC ?= yacc
YFLAGS ?= -d -t -v -y
CC ?= cc
CFLAGS ?= -std=c99 -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L -D_REENTRANT \
	-Wall -Wdisabled-optimization -Wpointer-arith -Wredundant-decls \
	-Wcast-qual -Wwrite-strings -Wtype-limits -Wundef -Wextra \
	-Winline -O0 -g

TOOLS = genconfigfile keydbparser

.PHONY: all clean $(TOOLS)
all:	$(TOOLS)

clean:
	@rm -f $(TOOLS) *.o y.tab.c y.tab.h y.output lex.yy.c

genconfigfile: genconfigfile.c
	gcc -o $@ $?

keydbparser: lex.yy.o y.tab.o
	$(CC) $(CFLAGS) -o $@ $^ -ly -ll

y.tab.o: y.tab.c
	$(CC) $(CFLAGS) -c -o $@ $<

lex.yy.o: lex.yy.c y.tab.h
	$(CC) $(CFLAGS) -c -o $@ $<

y.tab.c y.tab.h: keydbcfg-parser.y
	$(YACC) $(YFLAGS) $<

lex.yy.c: keydbcfg-lexer.l
	$(LEX) $(LFLAGS) $<
