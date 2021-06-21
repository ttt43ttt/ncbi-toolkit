# $Id: Makefile.test_psg_cache.app 568484 2018-08-06 17:02:17Z dmitrie1 $

APP = test_psg_cache

SRC = test_psg_cache

CPPFLAGS = $(LMDB_INCLUDE) $(PROTOBUF_INCLUDE) $(ORIG_CPPFLAGS)
LIBS = $(LMDB_LIBS) $(PROTOBUF_LIBS) $(ORIG_LIBS)
LIB = $(SEQ_LIBS) pub medline biblio general psg_protobuf psg_cache  xser xutil xncbi 

REQUIRES = MT Linux LMDB PROTOBUF

WATCHERS = satskyse dmitrie1
