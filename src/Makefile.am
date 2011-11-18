LIB_VERSION_INFO="@LT_VERSION_INFO@"

SUBDIRS= . examples

SET_FEATURES = -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L -D_REENTRANT
SET_WARNINGS = @SET_WARNINGS@
SET_OPTIMIZATIONS = @SET_OPTIMIZATIONS@
SET_DEBUG_OPTS = @SET_DEBUG_OPTS@
SET_INCLUDES = -I$(top_srcdir) -Ifile

AM_CFLAGS = -std=c99 $(SET_FEATURES) $(SET_WARNINGS) $(SET_OPTIMIZATIONS) \
	$(SET_DEBUG_OPTS) $(SET_INCLUDES) $(LIBGCRYPT_CFLAGS)
AM_YFLAGS = -d -p libaacs_yy
CFLAGS=$(shell test -z "$$CFLAGS" || echo "$$CFLAGS")

lib_LTLIBRARIES = libaacs.la
libaacs_la_SOURCES=libaacs/aacs.h \
	libaacs/mmc.h \
	libaacs/mkb.h \
	libaacs/aacs.c \
	libaacs/mmc.c \
	libaacs/crypto.c \
	libaacs/crypto.h \
	libaacs/mkb.c \
	file/file.h \
	file/filesystem.h \
	file/file_posix.c \
	file/keydbcfg.c \
	file/keydbcfg.h \
	file/keydbcfg-parser.y \
	file/keydbcfg-lexer.l \
	util/attributes.h \
	util/macro.h \
	util/logging.c \
	util/logging.h \
	util/strutl.c \
	util/strutl.h

if HAVE_WIN32
libaacs_la_SOURCES+= \
	file/win32.c \
	file/win32.h
else
libaacs_la_SOURCES+= \
	file/xdg.c \
	file/xdg.h
endif

libaacs_ladir= $(includedir)/libaacs
libaacs_la_HEADERS= libaacs/aacs.h file/filesystem.h libaacs/aacs-version.h
libaacs_la_LDFLAGS= $(LIBGCRYPT_LIBS) -version-info $(LIB_VERSION_INFO)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libaacs.pc

BUILT_SOURCES = keydbcfg-parser.h keydbcfg-parser.c keydbcfg-lexer.c

clean-lex-yacc:
	-rm -f $(BUILT_SOURCES)
