# $Id: Makefile.concat_seqentries.app 502386 2016-05-24 14:36:14Z gouriano $

APP = concat_seqentries
SRC = concat_seqentries

LIB = asn_cache  seqset $(SEQ_LIBS) pub medline biblio general \
	  bdb xser xconnect \
	  $(COMPRESS_LIBS) xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = marksc2
