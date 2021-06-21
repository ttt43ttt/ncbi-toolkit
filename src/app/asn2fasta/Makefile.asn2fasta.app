#################################
# $Id: Makefile.asn2fasta.app 536927 2017-05-24 16:00:07Z ucko $
# Author:  Mati Shomrat
#################################

# Build application "asn2fasta"
#################################

APP = asn2fasta
SRC = asn2fasta
LIB = $(ncbi_xloader_wgs) $(SRAREAD_LIBS) \
          xobjwrite variation_utils $(OBJREAD_LIBS) $(XFORMAT_LIBS) \
          xalnmgr xobjutil valerr xregexp \
	  ncbi_xdbapi_ftds dbapi $(ncbi_xreader_pubseqos2) $(FTDS_LIB) \
          entrez2cli entrez2 tables $(OBJMGR_LIBS) xregexp $(PCRE_LIB)

LIBS = $(FTDS_LIBS) $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) \
	   $(DL_LIBS) $(PCRE_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects


WATCHERS = ludwigf foleyjp
