### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=201001L -D_XOPEN_SOURCE=800 -D_BSD_SOURCE
AM_CPPFLAGS += -D__STDC_WANT_DEC_FP__
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

CLEANFILES += version.c
EXTRA_DIST += version.c.in

noinst_LTLIBRARIES += libcattle.la
libcattle_la_SOURCES = cattle.h
libcattle_la_SOURCES += cattle-ratio.c cattle-ratio.h
libcattle_la_SOURCES += cattle-price.c cattle-price.h
libcattle_la_SOURCES += cattle-quant.c cattle-quant.h
libcattle_la_SOURCES += cattle-perio.h
libcattle_la_SOURCES += cattle-date.h
libcattle_la_SOURCES += caev.c caev.h
libcattle_la_SOURCES += dt-strpf.c dt-strpf.h
libcattle_la_SOURCES += instant.h
libcattle_la_SOURCES += wheap.c wheap.h
libcattle_la_SOURCES += version.c version.h
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
cattle_SOURCES += cattle.ggo
cattle_CPPFLAGS = $(AM_CPPFLAGS)
cattle_CPPFLAGS += -DSTANDALONE
cattle_LDADD = libcattle.la
if USE_ASM_CORUS
cattle_LDADD += libcoru.la
endif  USE_ASM_CORUS
BUILT_SOURCES += cattle.x cattle.xh


analysis: $(libcattle_la_SOURCES:%.c=%.sa)

version.c: version.c.in $(top_builddir)/version.mk
	sed -e 's,[@]VERSION[@],$(VERSION),g' <$(srcdir)/$@.in >$@

## gperf rule
%.c: %.erf
	$(AM_V_GEN) $(GPERF) -L ANSI-C $< --output-file $@

## gengetopt rule with sed hack to avoid using VERSION and config.h
## depend on Makefile.am to survive a git pull
%.x %.xh: %.ggo Makefile.am
	$(AM_V_GEN) gengetopt -l -c x -H xh -i $< -F $* \
	&& sed \
	  -e 's,config.h,version.h,g' \
	  -e 's,\(.*\)_PARSER_VERSION VERSION,\1_PARSER_VERSION $(PACKAGE)_version_string,g' \
	  $*.xh >$*.xh-t \
	&& mv $*.xh-t $*.xh

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
%.sa: %.c
	$(AM_V_GEN) clang -std=gnu99 --analyze $< -o $@
