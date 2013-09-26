# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_CPPFLAGS += -DTEST

EXTRA_DIST = $(BUILT_SOURCES) $(TESTS)
TESTS =
TEST_EXTENSIONS =
BUILT_SOURCES =
dt_tests =
bin_tests =
batch_tests =
check_PROGRAMS =
CLEANFILES = $(check_PROGRAMS)

TEST_EXTENSIONS += .clit
TESTS_ENVIRONMENT = root=$(top_srcdir)
CLIT_LOG_COMPILER = $(builddir)/clitoris
AM_CLIT_LOG_FLAGS = -v --builddir $(top_builddir)/src
AM_LOG_COMPILER = false

EXTRA_DIST += caev-io.c
cattle_CFLAGS = -I$(top_srcdir)/src
cattle_LIBS = $(top_builddir)/src/libcattle.la

check_PROGRAMS += caev.01
caev_01_SOURCES = caev.01.c
caev_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_01_LDADD = $(cattle_LIBS)
TESTS += caev.01.clit

check_PROGRAMS += caev.02
caev_02_SOURCES = caev.02.c
caev_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_02_LDADD = $(cattle_LIBS)
TESTS += caev.02.clit

check_PROGRAMS += caev.03
caev_03_SOURCES = caev.03.c
caev_03_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_03_LDADD = $(cattle_LIBS)
TESTS += caev.03.clit

check_PROGRAMS += caev.04
caev_04_SOURCES = caev.04.c
caev_04_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_04_LDADD = $(cattle_LIBS)
TESTS += caev.04.clit

check_PROGRAMS += caev.05
caev_05_SOURCES = caev.05.c
caev_05_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_05_LDADD = $(cattle_LIBS)
TESTS += caev.05.clit

check_PROGRAMS += caev.06
caev_06_SOURCES = caev.06.c
caev_06_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_06_LDADD = $(cattle_LIBS)
TESTS += caev.06.clit

check_PROGRAMS += caev.07
caev_07_SOURCES = caev.07.c
caev_07_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_07_LDADD = $(cattle_LIBS)
TESTS += caev.07.clit

check_PROGRAMS += caev.08
caev_08_SOURCES = caev.08.c
caev_08_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_08_LDADD = $(cattle_LIBS)
TESTS += caev.08.clit

check_PROGRAMS += caev.09
caev_09_SOURCES = caev.09.c
caev_09_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_09_LDADD = $(cattle_LIBS)
TESTS += caev.09.clit

check_PROGRAMS += caev.10
caev_10_SOURCES = caev.10.c
caev_10_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_10_LDADD = $(cattle_LIBS)
TESTS += caev.10.clit

check_PROGRAMS += caev_pact.01
caev_pact_01_SOURCES = caev_pact.01.c
caev_pact_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_pact_01_LDADD = $(cattle_LIBS)
TESTS += caev_pact.01.clit

check_PROGRAMS += caev_pact.02
caev_pact_02_SOURCES = caev_pact.02.c
caev_pact_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_pact_02_LDADD = $(cattle_LIBS)
TESTS += caev_pact.02.clit

check_PROGRAMS += caev_qact.01
caev_qact_01_SOURCES = caev_qact.01.c
caev_qact_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_qact_01_LDADD = $(cattle_LIBS)
TESTS += caev_qact.01.clit

check_PROGRAMS += caev_qact.02
caev_qact_02_SOURCES = caev_qact.02.c
caev_qact_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_qact_02_LDADD = $(cattle_LIBS)
TESTS += caev_qact.02.clit

check_PROGRAMS += caev-supp.01
caev_supp_01_SOURCES = caev-supp.01.c
caev_supp_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_01_LDADD = $(cattle_LIBS)
TESTS += caev-supp.01.clit

check_PROGRAMS += caev-supp.02
caev_supp_02_SOURCES = caev-supp.02.c
caev_supp_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_02_LDADD = $(cattle_LIBS)
TESTS += caev-supp.02.clit

check_PROGRAMS += caev-supp.03
caev_supp_03_SOURCES = caev-supp.03.c
caev_supp_03_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_03_LDADD = $(cattle_LIBS)
TESTS += caev-supp.03.clit

check_PROGRAMS += caev-supp.04
caev_supp_04_SOURCES = caev-supp.04.c
caev_supp_04_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_04_LDADD = $(cattle_LIBS)
TESTS += caev-supp.04.clit

check_PROGRAMS += caev-supp.05
caev_supp_05_SOURCES = caev-supp.05.c
caev_supp_05_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_05_LDADD = $(cattle_LIBS)
TESTS += caev-supp.05.clit

check_PROGRAMS += caev-supp.06
caev_supp_06_SOURCES = caev-supp.06.c
caev_supp_06_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_06_LDADD = $(cattle_LIBS)
TESTS += caev-supp.06.clit

check_PROGRAMS += caev-supp.07
caev_supp_07_SOURCES = caev-supp.07.c
caev_supp_07_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_07_LDADD = $(cattle_LIBS)
TESTS += caev-supp.07.clit

check_PROGRAMS += caev-supp.08
caev_supp_08_SOURCES = caev-supp.08.c
caev_supp_08_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_08_LDADD = $(cattle_LIBS)
TESTS += caev-supp.08.clit

check_PROGRAMS += caev-supp.09
caev_supp_09_SOURCES = caev-supp.09.c
caev_supp_09_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_09_LDADD = $(cattle_LIBS)
TESTS += caev-supp.09.clit

check_PROGRAMS += caev-supp.10
caev_supp_10_SOURCES = caev-supp.10.c
caev_supp_10_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_10_LDADD = $(cattle_LIBS)
TESTS += caev-supp.10.clit

check_PROGRAMS += caev-supp.11
caev_supp_11_SOURCES = caev-supp.11.c
caev_supp_11_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_11_LDADD = $(cattle_LIBS)
TESTS += caev-supp.11.clit

check_PROGRAMS += caev-supp.12
caev_supp_12_SOURCES = caev-supp.12.c
caev_supp_12_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_12_LDADD = $(cattle_LIBS)
TESTS += caev-supp.12.clit

check_PROGRAMS += caev-supp.13
caev_supp_13_SOURCES = caev-supp.13.c
caev_supp_13_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_13_LDADD = $(cattle_LIBS)
TESTS += caev-supp.13.clit

check_PROGRAMS += caev-supp.14
caev_supp_14_SOURCES = caev-supp.14.c
caev_supp_14_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_14_LDADD = $(cattle_LIBS)
TESTS += caev-supp.14.clit

check_PROGRAMS += wheap_01
wheap_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
wheap_01_LDADD = $(cattle_LIBS)
TESTS += wheap_01.clit
TESTS += wheap_02.clit
TESTS += wheap_03.clit
TESTS += wheap_04.clit
TESTS += wheap_05.clit
TESTS += wheap_06.clit
TESTS += wheap_07.clit
TESTS += wheap_08.clit

check_PROGRAMS += wheap_09
wheap_09_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
wheap_09_LDADD = $(cattle_LIBS)
TESTS += wheap_09.clit
TESTS += wheap_10.clit
TESTS += wheap_11.clit
TESTS += wheap_12.clit
TESTS += wheap_13.clit
TESTS += wheap_14.clit
TESTS += wheap_15.clit
TESTS += wheap_16.clit

TESTS += cattle_01.clit
TESTS += cattle_02.clit
TESTS += cattle_03.clit
TESTS += cattle_04.clit
EXTRA_DIST += cattle_01.tser
EXTRA_DIST += cattle_01.echs

TESTS += cattle_appl_multi_01.clit
TESTS += cattle_appl_multi_02.clit
EXTRA_DIST += cattle_02.tser
EXTRA_DIST += cattle_03.tser

TESTS += cattle_appl_reverse_01.clit
TESTS += cattle_appl_reverse_02.clit


## our friendly helpers
check_PROGRAMS += clitoris
clitoris_CPPFLAGS = $(AM_CPPFLAGS)
clitoris_CPPFLAGS += -D_GNU_SOURCE
clitoris_LDFLAGS = $(AM_LDFLAGS) -static
clitoris_LDADD = -lutil
BUILT_SOURCES += clitoris.x clitoris.xh


## ggo rule
%.x %.xh: %.ggo
	$(AM_V_GEN) gengetopt -F $* -c x -H xh -i $<
	@sed -e 's,_PARSER_VERSION VERSION,_PARSER_VERSION "$(VERSION)",g' \
	     $*.xh > $*.xh-t \
	&& mv $*.xh-t $*.xh

## Makefile.am ends here
