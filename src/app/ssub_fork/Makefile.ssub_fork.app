#################################
# $Id: Makefile.ssub_fork.app 485044 2015-11-18 14:56:40Z foleyjp $
# Author:  Justin Foley
#################################

# Build application "seqsub_split"
#################################

APP = seqsub_split
SRC = ssub_fork
LIB = xobjwrite variation_utils $(OBJREAD_LIBS) xalnmgr xobjutil \
      gbseq entrez2cli entrez2 tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin


WATCHERS = foleyjp
