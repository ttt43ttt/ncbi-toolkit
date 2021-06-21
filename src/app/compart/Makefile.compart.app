# $Id: Makefile.compart.app 553549 2017-12-18 21:24:39Z fongah2 $
#################################
# Build demo "compart"

APP = compart
SRC = compart

LIB =  xalgoalignsplign xalgoalignutil xalgoalignnw xalgoseq taxon1 \
       $(BLAST_LIBS:%=%$(STATIC)) xqueryparse xregexp $(PCRE_LIB) \
       $(OBJMGR_LIBS:%=%$(STATIC))

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

CXXFLAGS = $(FAST_CXXFLAGS)
LDFLAGS  = $(FAST_LDFLAGS)

REQUIRES = -Cygwin

WATCHERS = chetvern
