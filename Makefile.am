SUBDIRS=src

ACLOCAL_AMFLAGS=-I m4

EXTRA_DIST=bootstrap COPYING KEYDB.cfg README.txt ChangeLog

AUTOMAKE_OPTIONS = \
	dist-bzip2 \
	no-dist-gzip

clean-lex-yacc:
	$(MAKE) -C src $@
