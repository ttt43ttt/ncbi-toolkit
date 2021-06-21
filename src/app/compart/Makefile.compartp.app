############################
# $Id: Makefile.compartp.app 553549 2017-12-18 21:24:39Z fongah2 $
############################

APP = compartp
SRC = compartp

LIB = prosplign xalgoalignutil xalgoseq taxon1 $(BLAST_LIBS) xqueryparse \
      xregexp $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

WATCHERS = chetvern
