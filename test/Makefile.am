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
TST_LOG_COMPILER = $(builddir)/clitoris
AM_TST_LOG_FLAGS = -v --builddir $(top_builddir)/src
AM_LOG_COMPILER = false

EXTRA_DIST += caev-io.c
cattle_CFLAGS = -I$(top_srcdir)/src
cattle_LIBS = $(top_builddir)/src/libcattle.la

check_PROGRAMS += caev.01
caev_01_SOURCES = caev.01.c
caev_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_01_LDADD = $(cattle_LIBS)
TESTS += caev.01
TESTS += caev.01.clit

check_PROGRAMS += caev.02
caev_02_SOURCES = caev.02.c
caev_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_02_LDADD = $(cattle_LIBS)
TESTS += caev.02
TESTS += caev.02.clit

check_PROGRAMS += caev.03
caev_03_SOURCES = caev.03.c
caev_03_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_03_LDADD = $(cattle_LIBS)
TESTS += caev.03
TESTS += caev.03.clit

check_PROGRAMS += caev.04
caev_04_SOURCES = caev.04.c
caev_04_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_04_LDADD = $(cattle_LIBS)
TESTS += caev.04
TESTS += caev.04.clit

check_PROGRAMS += caev.05
caev_05_SOURCES = caev.05.c
caev_05_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_05_LDADD = $(cattle_LIBS)
TESTS += caev.05
TESTS += caev.05.clit

check_PROGRAMS += caev.06
caev_06_SOURCES = caev.06.c
caev_06_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_06_LDADD = $(cattle_LIBS)
TESTS += caev.06
TESTS += caev.06.clit

check_PROGRAMS += caev_pact.01
caev_pact_01_SOURCES = caev_pact.01.c
caev_pact_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_pact_01_LDADD = $(cattle_LIBS)
TESTS += caev_pact.01
TESTS += caev_pact.01.clit

check_PROGRAMS += caev_pact.02
caev_pact_02_SOURCES = caev_pact.02.c
caev_pact_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_pact_02_LDADD = $(cattle_LIBS)
TESTS += caev_pact.02
TESTS += caev_pact.02.clit

check_PROGRAMS += caev_qact.01
caev_qact_01_SOURCES = caev_qact.01.c
caev_qact_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_qact_01_LDADD = $(cattle_LIBS)
TESTS += caev_qact.01
TESTS += caev_qact.01.clit

check_PROGRAMS += caev_qact.02
caev_qact_02_SOURCES = caev_qact.02.c
caev_qact_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_qact_02_LDADD = $(cattle_LIBS)
TESTS += caev_qact.02
TESTS += caev_qact.02.clit


## our friendly helpers
check_PROGRAMS += clitoris
clitoris_CPPFLAGS = $(AM_CPPFLAGS)
clitoris_CPPFLAGS += -D_GNU_SOURCE
clitoris_LDFLAGS = $(AM_LDFLAGS) -static
clitoris_LDADD = -lutil
BUILT_SOURCES += clitoris.x clitoris.h


## ggo rule
%.x %.h: %.ggo
	$(AM_V_GEN) gengetopt -F $* -c x -i $<
	@sed -e 's,_PARSER_VERSION VERSION,_PARSER_VERSION "$(VERSION)",g' \
	     $*.h > $*.h-t \
	&& mv $*.h-t $*.h
## Makefile.am ends here
