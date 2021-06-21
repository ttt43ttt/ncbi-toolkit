# $Id: Makefile.segmasker.app 553731 2017-12-20 19:59:17Z fongah2 $

REQUIRES = objects algo

ASN_DEP = seq

APP = segmasker
SRC = segmasker

LIB_ = xobjsimple xalgosegmask $(BLAST_LIBS) $(OBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))

CPPFLAGS = $(ORIG_CPPFLAGS) $(BLAST_THIRD_PARTY_INCLUDE)
LIBS = $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)


WATCHERS = camacho fongah2
