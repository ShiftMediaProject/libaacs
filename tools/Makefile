TOOLS = genkeyfile genconfigfile

.PHONY: all clean $(TOOLS)
all:	$(TOOLS)

genkeyfile: genkeyfile.c
	gcc -o $@ $?

genconfigfile: genkeyfile.c
	gcc -o $@ $?

