TOOLS = genkeyfile genconfigfile

.PHONY: all clean $(TOOLS)
all:	$(TOOLS)

clean:
	@rm -f $(TOOLS)

genkeyfile: genkeyfile.c
	gcc -o $@ $?

genconfigfile: genconfigfile.c
	gcc -o $@ $?

