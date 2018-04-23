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
AM_CPPFLAGS += -I$(top_srcdir)/src

EXTRA_DIST = $(BUILT_SOURCES) $(ctests)
TESTS = $(btests) $(ctests)
TEST_EXTENSIONS =
BUILT_SOURCES =
ctests =
btests =
check_PROGRAMS =
CLEANFILES = $(check_PROGRAMS)
noinst_LIBRARIES =

## summon our helper
include clitoris.am
AM_CLIT_LOG_FLAGS = -v --builddir $(top_builddir)/src
TESTS_ENVIRONMENT = root=$(top_srcdir)

EXTRA_DIST += caev-io.c
cattle_CFLAGS = -I$(top_srcdir)/src
cattle_LIBS = $(top_builddir)/src/libcattle.la

check_PROGRAMS += caev.01
caev_01_SOURCES = caev.01.c
caev_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_01_LDADD = $(cattle_LIBS)
ctests += caev.01.clit

check_PROGRAMS += caev.02
caev_02_SOURCES = caev.02.c
caev_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_02_LDADD = $(cattle_LIBS)
ctests += caev.02.clit

check_PROGRAMS += caev.03
caev_03_SOURCES = caev.03.c
caev_03_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_03_LDADD = $(cattle_LIBS)
ctests += caev.03.clit

check_PROGRAMS += caev.04
caev_04_SOURCES = caev.04.c
caev_04_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_04_LDADD = $(cattle_LIBS)
ctests += caev.04.clit

check_PROGRAMS += caev.05
caev_05_SOURCES = caev.05.c
caev_05_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_05_LDADD = $(cattle_LIBS)
ctests += caev.05.clit

check_PROGRAMS += caev.06
caev_06_SOURCES = caev.06.c
caev_06_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_06_LDADD = $(cattle_LIBS)
ctests += caev.06.clit

check_PROGRAMS += caev.07
caev_07_SOURCES = caev.07.c
caev_07_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_07_LDADD = $(cattle_LIBS)
ctests += caev.07.clit

check_PROGRAMS += caev.08
caev_08_SOURCES = caev.08.c
caev_08_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_08_LDADD = $(cattle_LIBS)
ctests += caev.08.clit

check_PROGRAMS += caev.09
caev_09_SOURCES = caev.09.c
caev_09_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_09_LDADD = $(cattle_LIBS)
ctests += caev.09.clit

check_PROGRAMS += caev.10
caev_10_SOURCES = caev.10.c
caev_10_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_10_LDADD = $(cattle_LIBS)
ctests += caev.10.clit

check_PROGRAMS += caev_pact.01
caev_pact_01_SOURCES = caev_pact.01.c
caev_pact_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_pact_01_LDADD = $(cattle_LIBS)
ctests += caev_pact.01.clit

check_PROGRAMS += caev_pact.02
caev_pact_02_SOURCES = caev_pact.02.c
caev_pact_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_pact_02_LDADD = $(cattle_LIBS)
ctests += caev_pact.02.clit

check_PROGRAMS += caev_qact.01
caev_qact_01_SOURCES = caev_qact.01.c
caev_qact_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_qact_01_LDADD = $(cattle_LIBS)
ctests += caev_qact.01.clit

check_PROGRAMS += caev_qact.02
caev_qact_02_SOURCES = caev_qact.02.c
caev_qact_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_qact_02_LDADD = $(cattle_LIBS)
ctests += caev_qact.02.clit

check_PROGRAMS += caev-supp.01
caev_supp_01_SOURCES = caev-supp.01.c
caev_supp_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_01_LDADD = $(cattle_LIBS)
ctests += caev-supp.01.clit

check_PROGRAMS += caev-supp.02
caev_supp_02_SOURCES = caev-supp.02.c
caev_supp_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_02_LDADD = $(cattle_LIBS)
ctests += caev-supp.02.clit

check_PROGRAMS += caev-supp.03
caev_supp_03_SOURCES = caev-supp.03.c
caev_supp_03_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_03_LDADD = $(cattle_LIBS)
ctests += caev-supp.03.clit

check_PROGRAMS += caev-supp.04
caev_supp_04_SOURCES = caev-supp.04.c
caev_supp_04_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_04_LDADD = $(cattle_LIBS)
ctests += caev-supp.04.clit

check_PROGRAMS += caev-supp.05
caev_supp_05_SOURCES = caev-supp.05.c
caev_supp_05_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_05_LDADD = $(cattle_LIBS)
ctests += caev-supp.05.clit

check_PROGRAMS += caev-supp.06
caev_supp_06_SOURCES = caev-supp.06.c
caev_supp_06_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_06_LDADD = $(cattle_LIBS)
ctests += caev-supp.06.clit

check_PROGRAMS += caev-supp.07
caev_supp_07_SOURCES = caev-supp.07.c
caev_supp_07_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_07_LDADD = $(cattle_LIBS)
ctests += caev-supp.07.clit

check_PROGRAMS += caev-supp.08
caev_supp_08_SOURCES = caev-supp.08.c
caev_supp_08_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_08_LDADD = $(cattle_LIBS)
ctests += caev-supp.08.clit

check_PROGRAMS += caev-supp.09
caev_supp_09_SOURCES = caev-supp.09.c
caev_supp_09_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_09_LDADD = $(cattle_LIBS)
ctests += caev-supp.09.clit

check_PROGRAMS += caev-supp.10
caev_supp_10_SOURCES = caev-supp.10.c
caev_supp_10_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_10_LDADD = $(cattle_LIBS)
ctests += caev-supp.10.clit

check_PROGRAMS += caev-supp.11
caev_supp_11_SOURCES = caev-supp.11.c
caev_supp_11_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_11_LDADD = $(cattle_LIBS)
ctests += caev-supp.11.clit

check_PROGRAMS += caev-supp.12
caev_supp_12_SOURCES = caev-supp.12.c
caev_supp_12_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_12_LDADD = $(cattle_LIBS)
ctests += caev-supp.12.clit

check_PROGRAMS += caev-supp.13
caev_supp_13_SOURCES = caev-supp.13.c
caev_supp_13_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_13_LDADD = $(cattle_LIBS)
ctests += caev-supp.13.clit

check_PROGRAMS += caev-supp.14
caev_supp_14_SOURCES = caev-supp.14.c
caev_supp_14_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
caev_supp_14_LDADD = $(cattle_LIBS)
ctests += caev-supp.14.clit

check_PROGRAMS += arith.01
arith_01_SOURCES = arith.01.c
arith_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
arith_01_LDADD = $(cattle_LIBS)
btests += arith.01

check_PROGRAMS += arith.02
arith_02_SOURCES = arith.02.c
arith_02_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
arith_02_LDADD = $(cattle_LIBS)
btests += arith.02

check_PROGRAMS += arith.03
arith_03_SOURCES = arith.03.c
arith_03_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
arith_03_LDADD = $(cattle_LIBS)
btests += arith.03

check_PROGRAMS += arith.04
arith_04_SOURCES = arith.04.c
arith_04_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
arith_04_LDADD = $(cattle_LIBS)
btests += arith.04

noinst_LIBRARIES += libwheap.a
libwheap_a_SOURCES = wheap.c
libwheap_a_SOURCES += dt-strpf.c

check_PROGRAMS += wheap_01
wheap_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
wheap_01_LDADD = libwheap.a
ctests += wheap_01.clit
ctests += wheap_02.clit
ctests += wheap_03.clit
ctests += wheap_04.clit
ctests += wheap_05.clit
ctests += wheap_06.clit
ctests += wheap_07.clit
ctests += wheap_08.clit

check_PROGRAMS += wheap_09
wheap_09_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
wheap_09_LDADD = libwheap.a
ctests += wheap_09.clit
ctests += wheap_10.clit
ctests += wheap_11.clit
ctests += wheap_12.clit
ctests += wheap_13.clit
ctests += wheap_14.clit
ctests += wheap_15.clit
ctests += wheap_16.clit


ctests += cattle_print_01.clit
ctests += cattle_print_02.clit
ctests += cattle_print_03.clit
ctests += cattle_print_04.clit
EXTRA_DIST += cattle_01.tser
EXTRA_DIST += cattle_01.echs

ctests += cattle_print_05.clit
ctests += cattle_print_06.clit
ctests += cattle_print_07.clit
ctests += cattle_print_08.clit
ctests += cattle_print_09.clit
ctests += cattle_print_10.clit
ctests += cattle_print_11.clit
ctests += cattle_print_12.clit
ctests += cattle_print_13.clit
ctests += cattle_print_14.clit
ctests += cattle_print_15.clit
ctests += cattle_print_16.clit
ctests += cattle_print_17.clit
ctests += cattle_print_18.clit
ctests += cattle_print_19.clit
ctests += cattle_print_20.clit
EXTRA_DIST += SVT.echs
EXTRA_DIST += XMT.echs

ctests += cattle_print_21.clit
ctests += cattle_print_22.clit
EXTRA_DIST += CBK.echs

ctests += cattle_apply_01.clit
ctests += cattle_apply_02.clit
ctests += cattle_apply_03.clit
ctests += cattle_apply_04.clit
ctests += cattle_apply_05.clit
ctests += cattle_apply_06.clit
ctests += cattle_apply_07.clit
ctests += cattle_apply_08.clit
EXTRA_DIST += cattle_05.tser
EXTRA_DIST += cattle_05.echs

ctests += cattle_apply_reverse_07.clit
ctests += cattle_apply_reverse_08.clit
ctests += cattle_apply_reverse_09.clit
ctests += cattle_apply_reverse_10.clit
ctests += cattle_apply_reverse_11.clit
ctests += cattle_apply_reverse_12.clit
EXTRA_DIST += cattle_03.tser

ctests += cattle_apply_13.clit
ctests += cattle_apply_14.clit
EXTRA_DIST += cattle_04.tser
EXTRA_DIST += cattle_04.echs
ctests += cattle_apply_15.clit
ctests += cattle_apply_16.clit
EXTRA_DIST += cattle_07.tser
EXTRA_DIST += cattle_07.echs
ctests += cattle_apply_17.clit
ctests += cattle_apply_18.clit
EXTRA_DIST += cattle_08.tser
EXTRA_DIST += cattle_08.echs
ctests += cattle_apply_19.clit
ctests += cattle_apply_20.clit
EXTRA_DIST += cattle_09.tser
EXTRA_DIST += cattle_09.echs
ctests += cattle_apply_21.clit
ctests += cattle_apply_22.clit
EXTRA_DIST += cattle_10.tser
EXTRA_DIST += cattle_10.echs

ctests += cattle_apply_23.clit
ctests += cattle_apply_24.clit
ctests += cattle_apply_25.clit
ctests += cattle_apply_26.clit
ctests += cattle_apply_27.clit
ctests += cattle_apply_28.clit
ctests += cattle_apply_29.clit
ctests += cattle_apply_30.clit
ctests += cattle_apply_31.clit
ctests += cattle_apply_32.clit
ctests += cattle_apply_33.clit
ctests += cattle_apply_34.clit
EXTRA_DIST += cattle_17.echs
EXTRA_DIST += cattle_23.echs
EXTRA_DIST += cattle_24.echs
EXTRA_DIST += cattle_25.echs
EXTRA_DIST += cattle_27.echs
EXTRA_DIST += cattle_28.echs
EXTRA_DIST += cattle_28.tser
EXTRA_DIST += cattle_29.echs
EXTRA_DIST += cattle_29.tser

ctests += cattle_apply_reverse_23.clit
ctests += cattle_apply_reverse_24.clit
ctests += cattle_apply_reverse_25.clit
ctests += cattle_apply_reverse_26.clit
ctests += cattle_apply_reverse_27.clit
ctests += cattle_apply_reverse_28.clit
ctests += cattle_apply_reverse_29.clit
ctests += cattle_apply_reverse_30.clit

ctests += cattle_apply_total_return_01.clit
ctests += cattle_apply_total_return_02.clit
ctests += cattle_apply_total_return_03.clit
ctests += cattle_apply_total_return_04.clit
ctests += cattle_apply_total_return_05.clit
ctests += cattle_apply_total_return_06.clit
ctests += cattle_apply_total_return_07.clit
ctests += cattle_apply_total_return_08.clit
ctests += cattle_apply_total_return_09.clit
ctests += cattle_apply_total_return_10.clit
ctests += cattle_apply_total_return_reverse_05.clit
ctests += cattle_apply_total_return_reverse_06.clit
ctests += cattle_apply_total_return_reverse_07.clit
ctests += cattle_apply_total_return_reverse_08.clit
ctests += cattle_apply_total_return_reverse_09.clit
ctests += cattle_apply_total_return_reverse_10.clit
ctests += cattle_apply_total_return_reverse_11.clit
ctests += cattle_apply_total_return_reverse_12.clit
EXTRA_DIST += cattle_12.tser
EXTRA_DIST += cattle_12.echs
EXTRA_DIST += cattle_13.tser
EXTRA_DIST += cattle_13.echs
EXTRA_DIST += cattle_16.tser
EXTRA_DIST += cattle_16.echs

ctests += cattle_apply_chain_01.clit
ctests += cattle_apply_chain_02.clit
ctests += cattle_apply_chain_03.clit
ctests += cattle_apply_chain_04.clit
EXTRA_DIST += cattle_11.tser
EXTRA_DIST += cattle_11.echs

ctests += cattle_midpnt_01.clit
ctests += cattle_midpnt_02.clit
ctests += cattle_midpnt_03.clit
ctests += cattle_midpnt_04.clit
EXTRA_DIST += cattle_13.quot

ctests += cattle_apply_prec_01.clit
ctests += cattle_apply_prec_02.clit
ctests += cattle_apply_prec_03.clit
ctests += cattle_apply_prec_04.clit
ctests += cattle_apply_prec_05.clit
ctests += cattle_apply_prec_06.clit
ctests += cattle_apply_prec_07.clit
ctests += cattle_apply_prec_08.clit
ctests += cattle_apply_prec_09.clit
ctests += cattle_apply_prec_10.clit
ctests += cattle_apply_prec_11.clit
ctests += cattle_apply_prec_12.clit

ctests += cattle_apply_intraday_01.clit
ctests += cattle_apply_intraday_02.clit
ctests += cattle_apply_intraday_03.clit
ctests += cattle_apply_intraday_04.clit
EXTRA_DIST += cattle_14.tser
EXTRA_DIST += cattle_15.tser

ctests += cattle_apply_stdin_01.clit
ctests += cattle_apply_stdin_02.clit
ctests += cattle_apply_stdin_03.clit
ctests += cattle_apply_stdin_04.clit

ctests += cattle_exp_01.clit
ctests += cattle_exp_02.clit
ctests += cattle_exp_03.clit
ctests += cattle_log_01.clit
ctests += cattle_log_02.clit
ctests += cattle_log_03.clit

ctests += cattle_voladj_01.clit
ctests += cattle_voladj_02.clit
ctests += cattle_voladj_03.clit
ctests += cattle_voladj_04.clit
EXTRA_DIST += cattle_26.tser
EXTRA_DIST += cattle_27.tser

ctests += invalid_rdte.clit
EXTRA_DIST += invalid_rdte.echs


check_PROGRAMS += intern.01
intern_01_SOURCES = intern.01.c
intern_01_CPPFLAGS = $(AM_CPPFLAGS) $(cattle_CFLAGS)
intern_01_LDADD = $(cattle_LIBS)
ctests += intern.01.clit


wheap.c: $(top_srcdir)/src/wheap.c
	cp $(top_srcdir)/src/wheap.c $@

dt-strpf.c: $(top_srcdir)/src/dt-strpf.c
	cp $(top_srcdir)/src/dt-strpf.c $@

dfp754_d32.c: $(top_srcdir)/src/dfp754_d32.c
	cp $(top_srcdir)/src/dfp754_d32.c $@

## Makefile.am ends here
