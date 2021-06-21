# $Id: Makefile.asn_cache_test.app 502386 2016-05-24 14:36:14Z gouriano $

APP = asn_cache_test
SRC = asn_cache_test

LIB = ncbi_xloader_asn_cache asn_cache \
      bdb xconnect $(COMPRESS_LIBS) $(SOBJMGR_LIBS)

LIBS = $(BERKELEYDB_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = marksc2
