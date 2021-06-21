# $Id: Makefile.id1_fetch.app 500573 2016-05-05 16:42:41Z gouriano $

APP = id1_fetch
SRC = id1_fetch
LIB = $(XFORMAT_LIBS) xalnmgr xobjutil id1cli entrez2cli entrez2 tables \
      xregexp $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin

WATCHERS = ucko
