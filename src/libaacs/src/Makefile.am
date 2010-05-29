AM_CFLAGS = -I$(top_srcdir)

lib_LTLIBRARIES = libaacs.la
libaacs_la_SOURCES=libaacs/aacs.c libaacs/crypto.c libaacs/mkb.c libaacs/mmc.c \
	file/configfile.c file/file_posix.c util/logging.c
libaacs_ladir= $(includedir)/libaacs
libaacs_la_HEADERS= libaacs/aacs.h
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libaacs.pc
