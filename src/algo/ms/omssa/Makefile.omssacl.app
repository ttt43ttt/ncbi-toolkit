# $Id: Makefile.omssacl.app 553797 2017-12-21 17:12:04Z fongah2 $
# Author:  Lewis Y. Geer

# Build application "omssacl"
#################################

WATCHERS = lewisg gorelenk

CXXFLAGS = $(FAST_CXXFLAGS) $(CMPRS_INCLUDE) $(STATIC_CXXFLAGS)

LDFLAGS  = $(FAST_LDFLAGS) $(STATIC_LDFLAGS) $(RUNPATH_ORIGIN)

APP = omssacl

SRC = omssacl

LIB = xomssa omssa pepXML blast composition_adjustment tables seqdb blastdb \
      xregexp $(PCRE_LIB) xconnect $(COMPRESS_LIBS) $(SOBJMGR_LIBS) $(LMDB_LIB)

LIBS = $(PCRE_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(CMPRS_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

