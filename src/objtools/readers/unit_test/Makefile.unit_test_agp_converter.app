# $Id: Makefile.unit_test_agp_converter.app 501626 2016-05-17 17:32:10Z kornbluh $

APP = unit_test_agp_converter
SRC = unit_test_agp_converter

LIB  = xunittestutil $(OBJREAD_LIBS) xobjutil test_boost $(SOBJMGR_LIBS)
LIBS = $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD  =
CHECK_COPY = agp_converter_test_cases

WATCHERS = bollin ludwigf
