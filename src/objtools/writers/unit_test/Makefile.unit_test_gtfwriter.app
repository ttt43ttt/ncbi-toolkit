# $Id: Makefile.unit_test_gtfwriter.app 500410 2016-05-04 15:43:09Z gouriano $

APP = unit_test_gtfwriter
SRC = unit_test_gtfwriter
LIB = xunittestutil xobjwrite variation_utils $(OBJREAD_LIBS) xalnmgr xobjutil gbseq entrez2cli entrez2 \
	tables test_boost $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = gtfwriter_test_cases

WATCHERS = ludwigf