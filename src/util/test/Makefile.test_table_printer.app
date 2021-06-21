# $Id: Makefile.test_table_printer.app 572148 2018-10-09 18:03:29Z ivanov $

APP = test_table_printer
SRC = test_table_printer

LIB  = xutil test_boost xncbi
LIBS = $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

REQUIRES = Boost.Test.Included

CHECK_CMD = test_table_printer

WATCHERS = kachalos

