# $Id: Makefile.unit_test_biosample_chk.app 541643 2017-07-20 02:49:33Z ucko $

APP = unit_test_biosample_chk
SRC = unit_test_biosample_chk

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = xvalidate xunittestutil $(OBJEDIT_LIBS) $(XFORMAT_LIBS) xalnmgr xobjutil \
       taxon1 valerr tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =
CHECK_COPY = 
CHECK_TIMEOUT = 3000

WATCHERS = bollin
