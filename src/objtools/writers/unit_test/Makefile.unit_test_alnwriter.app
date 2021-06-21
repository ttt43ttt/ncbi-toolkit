# $Id: Makefile.unit_test_alnwriter.app 534455 2017-04-27 15:21:59Z foleyjp $

APP = unit_test_alnwriter
SRC = unit_test_alnwriter
LIB = xunittestutil xobjwrite variation_utils $(OBJEDIT_LIBS) $(OBJREAD_LIBS) xalnmgr xobjutil gbseq entrez2cli entrez2 \
        tables test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = alnwriter_test_cases

WATCHERS = foleyjp
