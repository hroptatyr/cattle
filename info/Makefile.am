# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

include $(top_builddir)/version.mk

BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)
EXTRA_DIST += $(man1_MANS)

man1_MANS =
man1_MANS += $(built_mans)
man1_MANS += cattle.man

BUILT_SOURCES += cattle.man
EXTRA_DIST += cattle.name
EXTRA_DIST += author.h2m

%.man: $(top_srcdir)/src/%.yuck $(top_srcdir)/configure $(top_builddir)/version.mk
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		yuck$(EXEEXT) genman --version=$(VERSION) -o $@ $<

## Makefile.am ends here
