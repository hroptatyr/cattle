# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_CPPFLAGS += -DTEST
AM_CPPFLAGS += -D__STDC_WANT_DEC_FP__

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

check_PROGRAMS += pr-d32_01
pr_d32_01_SOURCES = pr-d32_01.c
pr_d32_01_SOURCES += $(top_srcdir)/src/ctl-dfp754.c $(top_srcdir)/src/ctl-dfp754.h
pr_d32_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
TESTS += pr-d32_01

check_PROGRAMS += pr-d32_02
pr_d32_02_SOURCES = pr-d32_02.c
pr_d32_02_SOURCES += $(top_srcdir)/src/ctl-dfp754.c $(top_srcdir)/src/ctl-dfp754.h
pr_d32_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
TESTS += pr-d32_02

check_PROGRAMS += rd-d32_01
rd_d32_01_SOURCES = rd-d32_01.c
rd_d32_01_SOURCES += $(top_srcdir)/src/ctl-dfp754.c $(top_srcdir)/src/ctl-dfp754.h
rd_d32_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
TESTS += rd-d32_01

check_PROGRAMS += rd-d32_02
rd_d32_02_SOURCES = rd-d32_02.c
rd_d32_02_SOURCES += $(top_srcdir)/src/ctl-dfp754.c $(top_srcdir)/src/ctl-dfp754.h
rd_d32_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
TESTS += rd-d32_02

check_PROGRAMS += rp-bid32_01
rp_bid32_01_SOURCES = rp-bid32_01.c
rp_bid32_01_SOURCES += $(top_srcdir)/src/ctl-dfp754.h
rp_bid32_01_SOURCES += $(top_srcdir)/src/ctl-dfp754.c
rp_bid32_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)

check_PROGRAMS += rp-dpd32_01
rp_dpd32_01_SOURCES = rp-dpd32_01.c
rp_dpd32_01_SOURCES += $(top_srcdir)/src/ctl-dfp754.h
rp_dpd32_01_SOURCES += $(top_srcdir)/src/ctl-dfp754.c
rp_dpd32_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)


TESTS += cattle_print_01.clit
TESTS += cattle_print_02.clit
TESTS += cattle_print_03.clit
TESTS += cattle_print_04.clit
EXTRA_DIST += cattle_01.tser
EXTRA_DIST += cattle_01.echs

TESTS += cattle_apply_01.clit
TESTS += cattle_apply_02.clit
TESTS += cattle_apply_03.clit
TESTS += cattle_apply_04.clit
TESTS += cattle_apply_05.clit
TESTS += cattle_apply_06.clit
EXTRA_DIST += cattle_05.tser
EXTRA_DIST += cattle_05.echs

TESTS += cattle_apply_reverse_07.clit
TESTS += cattle_apply_reverse_08.clit
TESTS += cattle_apply_reverse_09.clit
TESTS += cattle_apply_reverse_10.clit
TESTS += cattle_apply_reverse_11.clit
TESTS += cattle_apply_reverse_12.clit
EXTRA_DIST += cattle_03.tser

TESTS += cattle_apply_13.clit
TESTS += cattle_apply_14.clit
EXTRA_DIST += cattle_04.tser
EXTRA_DIST += cattle_04.echs
TESTS += cattle_apply_15.clit
TESTS += cattle_apply_16.clit
EXTRA_DIST += cattle_07.tser
EXTRA_DIST += cattle_07.echs
TESTS += cattle_apply_17.clit
TESTS += cattle_apply_18.clit
EXTRA_DIST += cattle_08.tser
EXTRA_DIST += cattle_08.echs
TESTS += cattle_apply_19.clit
TESTS += cattle_apply_20.clit
EXTRA_DIST += cattle_09.tser
EXTRA_DIST += cattle_09.echs
TESTS += cattle_apply_21.clit
TESTS += cattle_apply_22.clit
EXTRA_DIST += cattle_10.tser
EXTRA_DIST += cattle_10.echs

TESTS += cattle_apply_reverse_23.clit
TESTS += cattle_apply_reverse_24.clit
TESTS += cattle_apply_reverse_25.clit
TESTS += cattle_apply_reverse_26.clit
TESTS += cattle_apply_reverse_27.clit
TESTS += cattle_apply_reverse_28.clit
TESTS += cattle_apply_reverse_29.clit
TESTS += cattle_apply_reverse_30.clit

TESTS += cattle_apply_total_return_01.clit
TESTS += cattle_apply_total_return_02.clit
TESTS += cattle_apply_total_return_03.clit
TESTS += cattle_apply_total_return_04.clit
TESTS += cattle_apply_total_return_05.clit
TESTS += cattle_apply_total_return_06.clit
TESTS += cattle_apply_total_return_07.clit
TESTS += cattle_apply_total_return_08.clit
TESTS += cattle_apply_total_return_reverse_05.clit
TESTS += cattle_apply_total_return_reverse_06.clit
TESTS += cattle_apply_total_return_reverse_07.clit
TESTS += cattle_apply_total_return_reverse_08.clit
TESTS += cattle_apply_total_return_reverse_09.clit
TESTS += cattle_apply_total_return_reverse_10.clit
TESTS += cattle_apply_total_return_reverse_11.clit
TESTS += cattle_apply_total_return_reverse_12.clit
EXTRA_DIST += cattle_12.tser
EXTRA_DIST += cattle_12.echs
EXTRA_DIST += cattle_13.tser
EXTRA_DIST += cattle_13.echs

TESTS += cattle_apply_chain_01.clit
TESTS += cattle_apply_chain_02.clit
TESTS += cattle_apply_chain_03.clit
TESTS += cattle_apply_chain_04.clit
EXTRA_DIST += cattle_11.tser
EXTRA_DIST += cattle_11.echs

TESTS += cattle_midpnt_01.clit
TESTS += cattle_midpnt_02.clit
TESTS += cattle_midpnt_03.clit
TESTS += cattle_midpnt_04.clit
EXTRA_DIST += cattle_13.quot

TESTS += cattle_apply_prec_01.clit
TESTS += cattle_apply_prec_02.clit
TESTS += cattle_apply_prec_03.clit
TESTS += cattle_apply_prec_04.clit
TESTS += cattle_apply_prec_05.clit
TESTS += cattle_apply_prec_06.clit
TESTS += cattle_apply_prec_07.clit
TESTS += cattle_apply_prec_08.clit
TESTS += cattle_apply_prec_09.clit
TESTS += cattle_apply_prec_10.clit
TESTS += cattle_apply_prec_11.clit
TESTS += cattle_apply_prec_12.clit


## our friendly helpers
check_PROGRAMS += clitoris
clitoris_CPPFLAGS = $(AM_CPPFLAGS)
clitoris_CPPFLAGS += -D_GNU_SOURCE
clitoris_LDFLAGS = $(AM_LDFLAGS) -static
clitoris_LDADD = -lutil
BUILT_SOURCES += clitoris.yucc


## yuck rule
SUFFIXES = .yucc .yuck
.yuck.yucc:
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		yuck$(EXEEXT) gen -o $@ $<

## Makefile.am ends here
