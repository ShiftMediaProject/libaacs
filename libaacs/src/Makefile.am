SET_FEATURES = -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L -D_REENTRANT
SET_WARNINGS = -Wall -Wdisabled-optimization -Wpointer-arith -Wredundant-decls \
	-Wcast-qual -Wwrite-strings -Wtype-limits -Wundef @WERROR@ \
	@EXTRA_WARNINGS@
SET_OPTIMIZATIONS = @OPTIMIZATIONS@
SET_INCLUDES = -I$(top_srcdir)

AM_CFLAGS = -std=c99 $(SET_FEATURES) $(SET_WARNINGS) $(SET_OPTIMIZATIONS) \
	$(SET_INCLUDES)
CFLAGS=$(shell test -z "$$CFLAGS" && echo "-g" || echo "$$CFLAGS")

lib_LTLIBRARIES = libaacs.la
libaacs_la_SOURCES=libaacs/aacs.c libaacs/crypto.c libaacs/mkb.c libaacs/mmc.c \
	file/configfile.c file/file_posix.c util/logging.c
libaacs_ladir= $(includedir)/libaacs
libaacs_la_HEADERS= libaacs/aacs.h
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libaacs.pc
