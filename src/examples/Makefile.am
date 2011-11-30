SET_FEATURES = -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L -D_REENTRANT
SET_WARNINGS = @SET_WARNINGS@
SET_OPTIMIZATIONS = @SET_OPTIMIZATIONS@
SET_DEBUG_OPTS = @SET_DEBUG_OPTS@
SET_INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/src/file

AM_CFLAGS = -std=c99 $(SET_FEATURES) $(SET_WARNINGS) $(SET_OPTIMIZATIONS) \
	$(SET_DEBUG_OPTS) $(SET_INCLUDES)
CFLAGS=$(shell test -z "$$CFLAGS" || echo "$$CFLAGS")

AACS_LIB = $(top_builddir)/src/libaacs.la

PARSER_TEST_DEPENDENCIES=$(top_srcdir)/src/keydbcfg-lexer.c $(top_srcdir)/src/keydbcfg-parser.c

noinst_PROGRAMS=parser_test libaacs_test

parser_test_SOURCE=parser_test.c
parser_test_LDADD = $(AACS_LIB) $(PARSER_TEST_DEPENDENCIES)

libaacs_test_SOURCE=libaacs_test.c
libaacs_test_LDADD = $(AACS_LIB)
