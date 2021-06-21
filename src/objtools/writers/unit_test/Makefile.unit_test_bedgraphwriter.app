# $Id: Makefile.unit_test_bedgraphwriter.app 522134 2016-12-15 16:13:05Z ludwigf $

APP = unit_test_bedgraphwriter
SRC = unit_test_bedgraphwriter
LIB = xunittestutil xobjwrite variation_utils $(OBJREAD_LIBS) xalnmgr xobjutil gbseq entrez2cli entrez2 \
        tables test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = bedgraphwriter_test_cases

WATCHERS = ludwigf