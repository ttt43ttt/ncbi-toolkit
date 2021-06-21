# $Id: Makefile.aln_build.app 502200 2016-05-23 14:42:40Z vakatov $

APP = aln_build
SRC = aln_build_app
LIB = xalnmgr xobjutil submit tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)


WATCHERS = grichenk
