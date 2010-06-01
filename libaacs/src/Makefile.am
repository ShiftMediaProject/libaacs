# library version info
# NOTE: this is not the same as the library's release number
LIB_CURRENT=0
LIB_REVISION=0
LIB_AGE=0
LIB_VERSION_INFO="$(LIB_CURRENT):$(LIB_REVISION):$(LIB_AGE)"

SET_FEATURES = -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L -D_REENTRANT
SET_WARNINGS = @SET_WARNINGS@
SET_OPTIMIZATIONS = @SET_OPTIMIZATIONS@
SET_INCLUDES = -I$(top_srcdir) -Ifile

AM_CFLAGS = -std=c99 $(SET_FEATURES) $(SET_WARNINGS) $(SET_OPTIMIZATIONS) \
	$(SET_INCLUDES)
CFLAGS=$(shell test -z "$$CFLAGS" && echo "-g" || echo "$$CFLAGS")

lib_LTLIBRARIES = libaacs.la
libaacs_la_SOURCES=libaacs/aacs.h \
	libaacs/mmc.h \
	libaacs/mkb.h \
	libaacs/aacs.c \
	libaacs/mmc.c \
	libaacs/crypto.c \
	libaacs/crypto.h \
	libaacs/mkb.c \
	file/configfile.c \
	file/configfile.h \
	file/file.h \
	file/file_posix.c \
	util/attributes.h \
	util/macro.h \
	util/logging.c \
	util/logging.h
libaacs_ladir= $(includedir)/libaacs
libaacs_la_HEADERS= libaacs/aacs.h file/configfile.h
libaacs_la_LDFLAGS= -version-info $(LIB_VERSION_INFO)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libaacs.pc
