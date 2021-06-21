# $Id: Makefile.unit_test_extended_cleanup.app 574562 2018-11-15 12:31:02Z ivanov $

APP = unit_test_extended_cleanup
SRC = unit_test_extended_cleanup

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)


LIB = xcleanup xobjedit xunittestutil xalnmgr xobjutil valid taxon3 gbseq submit \
      tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = bollin kans
