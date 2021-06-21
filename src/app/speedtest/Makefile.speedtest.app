#################################
# $Id: Makefile.speedtest.app 574560 2018-11-15 12:30:34Z ivanov $
# Author:  Frank Ludwig
#################################

# Build application "speedtest"
#################################

APP = speedtest
SRC = speedtest
LIB = prosplign xalgoalignutil taxon1 xalgoseq xcleanup xobjedit taxon3 valid valerr \
      $(BLAST_LIBS) xqueryparse xregexp $(PCRE_LIB) $(OBJMGR_LIBS:%=%$(STATIC))

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(PCRE_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

REQUIRES = objects algo -Cygwin


WATCHERS = ludwigf
