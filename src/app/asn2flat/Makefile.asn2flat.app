#################################
# $Id: Makefile.asn2flat.app 579394 2019-02-01 21:33:41Z fukanchi $
# Author:  Mati Shomrat
#################################

# Build application "asn2flat"
#################################

APP = asn2flat
SRC = asn2flat

LIB  = $(OBJREAD_LIBS) $(XFORMAT_LIBS) valerr \
       $(ncbi_xloader_wgs) $(SRAREAD_LIBS) \
           xalnmgr xobjutil entrez2cli entrez2 tables \
	   ncbi_xdbapi_ftds dbapi $(ncbi_xreader_pubseqos2) $(FTDS_LIB) \
	   xregexp $(PCRE_LIB)  $(SRAREAD_LIBS) $(DATA_LOADERS_UTIL_LIB) $(OBJMGR_LIBS)
LIBS = $(DATA_LOADERS_UTIL_LIBS) $(VDB_LIBS) $(FTDS_LIBS) $(CMPRS_LIBS) $(PCRE_LIBS) $(NETWORK_LIBS) \
	   $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects BerkeleyDB SQLITE3

WATCHERS = ludwigf gotvyans
