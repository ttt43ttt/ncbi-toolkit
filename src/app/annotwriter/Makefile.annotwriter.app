#################################
# $Id: Makefile.annotwriter.app 536927 2017-05-24 16:00:07Z ucko $
# Author:  Mati Shomrat
#################################

# Build application "annotwriter"
#################################

APP = annotwriter
SRC = annotwriter
LIB = xobjwrite $(XFORMAT_LIBS) variation_utils $(OBJEDIT_LIBS) xalnmgr \
      xobjutil entrez2cli entrez2 tables xregexp $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin


WATCHERS = ludwigf
