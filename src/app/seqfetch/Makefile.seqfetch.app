#################################
# $Id: Makefile.seqfetch.app 433380 2014-04-24 17:06:07Z ucko $
# Author:  Mati Shomrat
#################################

# Build application "seqfetch"
#################################

APP = seqfetch
SRC = seqfetch
LIB = xobjwrite variation_utils $(OBJREAD_LIBS) xalnmgr xobjutil \
      gbseq entrez2cli entrez2 tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin


WATCHERS = ludwigf
