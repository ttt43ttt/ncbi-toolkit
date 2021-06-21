#################################
# $Id: Makefile.asnval.app 541636 2017-07-19 23:04:41Z vakatov $
# Author:  Colleen Bollin, based on file by Mati Shomrat
#################################

# Build application "asnval"
#################################

APP = asnvalidate
SRC = asnval
LIB = xvalidate taxon1 xmlwrapp $(OBJEDIT_LIBS) $(XFORMAT_LIBS) xalnmgr xobjutil \
      valerr tables xregexp $(PCRE_LIB) $(DATA_LOADERS_UTIL_LIB) $(OBJMGR_LIBS)
LIBS = $(LIBXSLT_LIBS) $(DATA_LOADERS_UTIL_LIBS) $(LIBXML_LIBS) $(PCRE_LIBS) \
       $(CMPRS_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects LIBXML LIBXSLT BerkeleyDB SQLITE3

CXXFLAGS += $(ORIG_CXXFLAGS)
LDFLAGS  += $(ORIG_LDFLAGS)

WATCHERS = bollin gotvyans
