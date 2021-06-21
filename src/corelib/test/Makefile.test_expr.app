# $Id: Makefile.test_expr.app 571656 2018-10-01 12:47:06Z ivanov $

APP = test_expr
SRC = test_expr

LIB = test_boost$(STATIC) xncbi

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIBS = $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_CMD =


WATCHERS = vakatov
