# $Id: Makefile.convert_seq.app 536927 2017-05-24 16:00:07Z ucko $

APP = convert_seq
SRC = convert_seq
LIB = $(ncbi_xloader_wgs) $(SRAREAD_LIBS) xobjwrite variation_utils \
      $(OBJREAD_LIBS) $(XFORMAT_LIBS) xalnmgr xobjutil tables xregexp \
      $(PCRE_LIB) $(OBJMGR_LIBS)

LIBS = $(SRA_SDK_SYSLIBS) $(PCRE_LIBS) $(CMPRS_LIBS) \
       $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects

WATCHERS = ucko drozdov foleyjp
