# $Id: Makefile.unit_test_seq.app 443959 2014-08-20 17:34:08Z vasilche $

APP = unit_test_seq
SRC = unit_test_seq

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = test_boost xobjutil $(OBJMGR_LIBS)
LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD  =

WATCHERS = vasilche