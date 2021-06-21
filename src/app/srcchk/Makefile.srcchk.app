#################################
# $Id: Makefile.srcchk.app 536927 2017-05-24 16:00:07Z ucko $
# Author: Frank Ludwig
#################################

# Build application "srcchk"
#################################

APP = srcchk
SRC = srcchk
LIB = xobjwrite $(XFORMAT_LIBS) variation_utils $(OBJREAD_LIBS) xalnmgr \
      xobjutil entrez2cli entrez2 tables xregexp $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin

WATCHERS = ludwigf
