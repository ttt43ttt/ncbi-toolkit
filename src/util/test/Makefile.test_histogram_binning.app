#################################
# $Id: Makefile.test_histogram_binning.app 572148 2018-10-09 18:03:29Z ivanov $

APP = test_histogram_binning
SRC = test_histogram_binning
LIB = xutil test_boost xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD =

WATCHERS = kachalos

