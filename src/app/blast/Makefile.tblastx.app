# $Id: Makefile.tblastx.app 553487 2017-12-18 14:23:38Z fongah2 $

WATCHERS = camacho madden fongah2

APP = tblastx
SRC = tblastx_app
LIB_ = $(BLAST_INPUT_LIBS) $(BLAST_LIBS) xregexp $(PCRE_LIB) $(OBJMGR_LIBS)
LIB = blast_app_util $(LIB_:%=%$(STATIC))

# De-universalize Mac builds to work around a PPC toolchain limitation
CFLAGS 	 = $(FAST_CXXFLAGS:ppc=i386) 
CXXFLAGS = $(FAST_CXXFLAGS:ppc=i386) 
LDFLAGS  = $(FAST_LDFLAGS:ppc=i386) 

CPPFLAGS = -DNCBI_MODULE=BLAST $(ORIG_CPPFLAGS) $(BLAST_THIRD_PARTY_INCLUDE)
LIBS = $(BLAST_THIRD_PARTY_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin

PROJ_TAG = gbench