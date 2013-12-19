include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

ACLOCAL_AMFLAGS = -I m4

SUBDIRS =
SUBDIRS += build-aux
SUBDIRS += src
SUBDIRS += info
SUBDIRS += test

EXTRA_DIST =
DISTCLEANFILES =

EXTRA_DIST += build-aux/git-version-gen
EXTRA_DIST += README.md

DISTCLEANFILES += version.mk

if USE_ASM_CORUS
DISTCHECK_CONFIGURE_FLAGS = --with-asm-backed-coroutines
endif  USE_ASM_CORUS

dist-hook:
	echo $(VERSION) > $(distdir)/version \
	&& touch -r $(top_builddir)/version.mk $(distdir)/version
