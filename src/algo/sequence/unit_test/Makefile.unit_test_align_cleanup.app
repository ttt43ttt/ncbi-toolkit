# $Id: Makefile.unit_test_align_cleanup.app 453148 2014-12-01 16:00:52Z chetvern $

APP = unit_test_align_cleanup

SRC = unit_test_align_cleanup

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB  = xalgoseq taxon1 xalnmgr xobjutil tables xregexp $(PCRE_LIB) test_boost $(OBJMGR_LIBS)
LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

# Uncomment if you do not want it to run automatically as part of
# "make check".
CHECK_CMD = unit_test_align_cleanup
CHECK_COPY =

WATCHERS = chetvern
