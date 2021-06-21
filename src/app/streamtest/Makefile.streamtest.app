#################################
# $Id: Makefile.streamtest.app 553549 2017-12-18 21:24:39Z fongah2 $
# Author:  Frank Ludwig
#################################

# Build application "streamtest"
#################################

APP = streamtest
SRC = streamtest
LIB = prosplign xalgoalignutil xalgoseq xmlwrapp eutils_client hydra_client \
      xvalidate xobjwrite xobjedit $(XFORMAT_LIBS) variation_utils \
      valerr taxon1 $(BLAST_LIBS) \
      $(ncbi_xloader_wgs) $(SRAREAD_LIBS) xqueryparse xregexp $(PCRE_LIB) \
      $(OBJMGR_LIBS)

LIBS = $(SRA_SDK_SYSLIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) \
      $(PCRE_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects algo LIBXSLT

WATCHERS = kans drozdov
