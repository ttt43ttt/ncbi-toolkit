# $Id: Makefile.test_ctransition_nlmzip.app 561334 2018-04-05 19:28:44Z ucko $

APP = test_ctransition_nlmzip
SRC = test_ctransition_nlmzip
LIB = ctransition_nlmzip ctransition xcompress $(CMPRS_LIB) xutil xncbi
LIBS = $(CMPRS_LIBS) $(ORIG_LIBS)
CPPFLAGS = $(ORIG_CPPFLAGS) $(CMPRS_INCLUDE)

CHECK_CMD = 

WATCHERS = ivanov
