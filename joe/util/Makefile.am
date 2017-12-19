AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = \
  unicode-8-0-0/Blocks.txt \
  unicode-8-0-0/CaseFolding.txt \
  unicode-8-0-0/EastAsianWidth.txt \
  unicode-8-0-0/UnicodeData.txt \
  unicode-9-0-0/Blocks.txt \
  unicode-9-0-0/CaseFolding.txt \
  unicode-9-0-0/EastAsianWidth.txt \
  unicode-9-0-0/UnicodeData.txt \
  unicode-10-0-0/Blocks.txt \
  unicode-10-0-0/CaseFolding.txt \
  unicode-10-0-0/EastAsianWidth.txt \
  unicode-10-0-0/UnicodeData.txt \
  README

noinst_PROGRAMS = termidx stringify uniproc checkwidths

termidx_SOURCES = termidx.c

stringify_SOURCES = stringify.c

uniproc_SOURCES = uniproc.c

checkwidths_SOURCES = checkwidths.c

distclean-local:
	rm -f *~
