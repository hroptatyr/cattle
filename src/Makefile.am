### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=201001L -D_XOPEN_SOURCE=800 -D_BSD_SOURCE
AM_CPPFLAGS += -DHAVE_VERSION_H
## we need some system's special capabilities
AM_CPPFLAGS += -D_ALL_SOURCE
AM_CPPFLAGS += -D_NETBSD_SOURCE
AM_LDFLAGS = $(XCCLDFLAGS)

bin_PROGRAMS =
noinst_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
pkglib_LTLIBRARIES =
noinst_HEADERS =
BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)
CLEANFILES = 
SUFFIXES =

noinst_LTLIBRARIES += libcattle.la
EXTRA_libcattle_la_SOURCES =
libcattle_la_SOURCES = cattle.h
libcattle_la_SOURCES += cattle-ratio.c cattle-ratio.h
libcattle_la_SOURCES += cattle-price.c cattle-price.h
libcattle_la_SOURCES += cattle-quant.c cattle-quant.h
libcattle_la_SOURCES += cattle-perio.h
libcattle_la_SOURCES += cattle-date.h
libcattle_la_SOURCES += caev.c caev.h
libcattle_la_SOURCES += dt-strpf.c dt-strpf.h
libcattle_la_SOURCES += instant.c instant.h
EXTRA_libcattle_la_SOURCES += wikisort.c
libcattle_la_SOURCES += wheap.c wheap.h
libcattle_la_SOURCES += intern.c intern.h
libcattle_la_SOURCES += caev-series.c caev-series.h
libcattle_la_SOURCES += nifty.h
libcattle_la_SOURCES += coru.h
## actual mt564 events
libcattle_la_SOURCES += caev-supp.c caev-supp.h
libcattle_la_SOURCES += caev--ctl1.c
libcattle_la_SOURCES += caev--bonu.c
libcattle_la_SOURCES += caev--drip.c
libcattle_la_SOURCES += caev--dvca.c
libcattle_la_SOURCES += caev--dvse.c
libcattle_la_SOURCES += caev--rhts.c
libcattle_la_SOURCES += caev--splf.c
libcattle_la_SOURCES += caev--splr.c
libcattle_la_SOURCES += caev-rdr.c caev-rdr.h
libcattle_la_SOURCES += caev-rdr-gp.erf
libcattle_la_SOURCES += caev-fld-gp.erf
## dfp754 utils
libcattle_la_SOURCES += ctl-dfp754.c ctl-dfp754.h
libcattle_la_CPPFLAGS = $(AM_CPPFLAGS)
libcattle_la_CPPFLAGS += $(dfp754_CFLAGS)
libcattle_la_CPPFLAGS += -DWHEAP_COLOUR_H='"caev-series.h"'
libcattle_la_LDFLAGS = $(AM_LDFLAGS)
libcattle_la_LDFLAGS += $(dfp754_LIBS)
BUILT_SOURCES += caev-rdr-gp.c
BUILT_SOURCES += caev-fld-gp.c

if USE_ASM_CORUS
noinst_LTLIBRARIES += libcoru.la
libcoru_la_SOURCES =
EXTRA_libcoru_la_SOURCES =
libcoru_la_SOURCES += coru.h
libcoru_la_SOURCES += coru/cocore.c coru/cocore.h
libcoru_la_SOURCES += coru/switch.c coru/switch.h
libcoru_la_SOURCES += coru/platform.h
libcoru_la_SOURCES += coru/valgrind.h
libcoru_la_SOURCES += coru/memcheck.h
EXTRA_libcoru_la_SOURCES += coru/switch-arm.c
EXTRA_libcoru_la_SOURCES += coru/switch-ppc_osx.c
EXTRA_libcoru_la_SOURCES += coru/switch-x86.c
EXTRA_libcoru_la_SOURCES += coru/switch-x86_64.c
endif  USE_ASM_CORUS


bin_PROGRAMS += cattle
cattle_SOURCES = cattle.c
cattle_SOURCES += cattle.yuck
cattle_SOURCES += version.c version.h version.c.in
cattle_CPPFLAGS = $(AM_CPPFLAGS)
cattle_CPPFLAGS += -DSTANDALONE
cattle_CPPFLAGS += $(dfp754_CFLAGS)
cattle_LDADD = libcattle.la
if USE_ASM_CORUS
cattle_LDADD += libcoru.la
endif  USE_ASM_CORUS
BUILT_SOURCES += cattle.yucc


analysis: $(libcattle_la_SOURCES:%.c=%.sa)

## version rules
version.c: version.c.in $(top_builddir)/.version
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		yuck$(EXEEXT) scmver --ignore-noscm --force -o $@ \
			--use-reference --reference $(top_builddir)/.version $<

## gperf rule
SUFFIXES += .erf
.erf.c:
	$(AM_V_GEN) $(GPERF) -L ANSI-C $< --output-file $@

SUFFIXES += .yuck
SUFFIXES += .yucc
.yuck.yucc:
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		yuck$(EXEEXT) gen -o $@ $<

## for dist or dist check we need binaries, so help2man can work
dist-hook: $(bin_PROGRAMS)


## Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<

## create static analysis
SUFFIXES += .sa
.c.sa:
	$(AM_V_GEN) clang -std=gnu99 --analyze $< -o $@
