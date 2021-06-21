#################################
# $Id: Makefile.idmapper.app 500573 2016-05-05 16:42:41Z gouriano $
# Author:  Frank Ludwig
#################################

# Build application "idmapper"
#################################

APP = idmapper
SRC = idmapper
LIB = xobjreadex $(OBJREAD_LIBS) $(XFORMAT_LIBS) xalnmgr xobjutil valerr \
          xregexp entrez2cli entrez2 tables $(OBJMGR_LIBS) $(PCRE_LIB)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(PCRE_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin


WATCHERS = ludwigf
