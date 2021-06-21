# $Id: Makefile.unit_test_vcfwriter.app 433493 2014-04-25 14:48:39Z filippov $

APP = unit_test_vcfwriter
SRC = unit_test_vcfwriter
LIB = xunittestutil xobjwrite variation_utils $(OBJREAD_LIBS) xalnmgr \
      xobjutil gbseq entrez2cli entrez2 tables test_boost $(OBJMGR_LIBS)

#LIB = xunittestutil xobjwrite $(OBJREAD_LIBS) xobjutil gbseq xalnmgr entrez2cli entrez2 \
#	tables test_boost $(OBJMGR_LIBS)  variation_utils variation xutil xncbi

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = vcfwriter_test_cases

WATCHERS = ludwigf
