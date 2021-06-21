# $Id: Makefile.unit_test_score_builder.app 443109 2014-08-11 17:54:59Z chetvern $

APP = unit_test_score_builder
SRC = unit_test_score_builder

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = xalnmgr xobjutil submit tables test_boost $(OBJMGR_LIBS)
LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

#CHECK_COPY = data

CHECK_CMD =

WATCHERS = grichenk
