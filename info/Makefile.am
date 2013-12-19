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

my_bld_man = $(__bld_man_$(V))
__bld_man_ = $(__bld_man_$(AM_DEFAULT_VERBOSITY))
__bld_man_0 = @echo "  H2M" $@;
cattle.man: $(top_srcdir)/src/cattle.yuck \
		$(top_builddir)/version.mk \
		cattle.name
	$(my_bld_man) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I "$(srcdir)/author.h2m" \
		-I "$(srcdir)/cattle.name" \
		-N \
		-o $@ $(top_builddir)/src/cattle

## Makefile.am ends here
