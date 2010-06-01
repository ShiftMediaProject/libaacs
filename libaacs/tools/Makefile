TOOLS = genconfigfile

.PHONY: all clean $(TOOLS)
all:	$(TOOLS)

clean:
	@rm -f $(TOOLS)

genconfigfile: genconfigfile.c
	gcc -o $@ $?
