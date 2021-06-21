# $Id: Makefile.hfilter.app 553549 2017-12-18 21:24:39Z fongah2 $

WATCHERS = badrazat

APP = hfilter
SRC = hitfilter_app

LIB = xalgoalignutil xalgoseq taxon1 $(BLAST_LIBS) \
	xqueryparse xregexp $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)
