SET_FEATURES = -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L
SET_INCLUDES = -I$(top_srcdir)

AM_CFLAGS = -std=c99 $(SET_FEATURES) $(SET_INCLUDES)

lib_LTLIBRARIES = libaacs.la
libaacs_la_SOURCES=libaacs/aacs.c libaacs/crypto.c libaacs/mkb.c libaacs/mmc.c \
	file/configfile.c file/file_posix.c util/logging.c
libaacs_ladir= $(includedir)/libaacs
libaacs_la_HEADERS= libaacs/aacs.h
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libaacs.pc
