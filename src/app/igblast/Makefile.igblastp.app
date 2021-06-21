# $Id: Makefile.igblastp.app 553549 2017-12-18 21:24:39Z fongah2 $

APP = igblastp
SRC = igblastp_app
LIB_ = xalgoalignutil $(BLAST_INPUT_LIBS) xqueryparse $(BLAST_LIBS) \
	xregexp $(PCRE_LIB) $(OBJMGR_LIBS)
LIB = blast_app_util igblast $(LIB_:%=%$(STATIC))

# De-universalize Mac builds to work around a PPC toolchain limitation
CFLAGS   = $(FAST_CFLAGS:ppc=i386)
CXXFLAGS = $(FAST_CXXFLAGS:ppc=i386)
LDFLAGS  = $(FAST_LDFLAGS:ppc=i386)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin

WATCHERS = camacho madden maning