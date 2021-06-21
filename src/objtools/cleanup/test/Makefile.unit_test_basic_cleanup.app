# $Id: Makefile.unit_test_basic_cleanup.app 574562 2018-11-15 12:31:02Z ivanov $

APP = unit_test_basic_cleanup
SRC = unit_test_basic_cleanup

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xcleanup xobjedit xunittestutil xobjutil valid taxon3 submit xconnect \
      xregexp $(PCRE_LIB) $(COMPRESS_LIBS) test_boost $(SOBJMGR_LIBS)

LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_cases
CHECK_TIMEOUT = 1200

WATCHERS = bollin kans 
