SET_FEATURES = -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L -D_REENTRANT
SET_INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/src/file

AM_CFLAGS = -std=c99 $(SET_FEATURES) $(SET_INCLUDES)

AACS_LIB = $(top_builddir)/src/libaacs.la

noinst_PROGRAMS=parser_test
bin_PROGRAMS=aacs_info

parser_test_SOURCES=parser_test.c \
	$(top_builddir)/src/keydbcfg-lexer.c \
	$(top_builddir)/src/keydbcfg-parser.c

parser_test_LDADD = $(AACS_LIB)

aacs_info_SOURCES=aacs_info.c
aacs_info_LDADD = $(AACS_LIB)
