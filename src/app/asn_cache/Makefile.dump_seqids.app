# $Id: Makefile.dump_seqids.app 502386 2016-05-24 14:36:14Z gouriano $

APP = dump_asn_cache_index
SRC = dump_seqids

LIB = asn_cache  seqset $(SEQ_LIBS) pub medline biblio general \
	  bdb xser xconnect \
	  $(COMPRESS_LIBS) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = marksc2
