#################################
# $Id: Makefile.prot_match.app 529115 2017-03-01 15:17:21Z foleyjp $
# Author:  Justin Foley
#################################

# Build application "prot_match"
#################################

APP = prot_match
SRC = prot_match run_binary

CPPFLAGS = $(ORIG_CPPFLAGS) 

LIB = protein_match xobjwrite $(OBJREAD_LIBS) xalnmgr xobjutil \
      gbseq entrez2cli entrez2 tables $(DATA_LOADERS_UTIL_LIB) $(OBJMGR_LIBS)

LIBS = $(DATA_LOADERS_UTIL_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = objects BerkeleyDB SQLITE3 -Cygwin MT

WATCHERS = foleyjp
