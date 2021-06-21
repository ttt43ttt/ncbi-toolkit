# $Id: Makefile.unit_test_capitalization_string.app 574563 2018-11-15 12:41:30Z ivanov $

APP = unit_test_capitalization_string
SRC = unit_test_capitalization_string

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xcleanup xobjedit xunittestutil xobjutil valid taxon3 xconnect \
      xregexp $(PCRE_LIB) $(COMPRESS_LIBS) test_boost $(SOBJMGR_LIBS)

LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = test_cases
CHECK_TIMEOUT = 1200

WATCHERS = bollin kans foleyjp
