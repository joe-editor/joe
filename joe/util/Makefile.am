AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = Blocks-8-0-0.txt CaseFolding-8-0-0.txt \
  EastAsianWidth-8-0-0.txt UnicodeData-8-0-0.txt README

noinst_PROGRAMS = termidx stringify uniproc

termidx_SOURCES = termidx.c

stringify_SOURCES = stringify.c

uniproc_SOURCES = uniproc.c

distclean-local:
	rm -f *~
