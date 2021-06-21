#################################
# $Id: Makefile.update_prot_id.app 506340 2016-07-06 18:51:56Z foleyjp $
# Author:  Justin Foley
#################################

# Build application "update_prot_id"
#################################

APP = update_prot_id
SRC = update_prot_id
LIB = xobjwrite variation_utils $(OBJREAD_LIBS) xalnmgr xobjutil \
      gbseq entrez2cli entrez2 tables $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects -Cygwin


WATCHERS = foleyjp
