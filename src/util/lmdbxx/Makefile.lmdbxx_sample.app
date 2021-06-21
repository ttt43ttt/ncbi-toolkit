# $Id: Makefile.lmdbxx_sample.app 542362 2017-07-30 00:39:23Z lavr $

REQUIRES = LMDB

SRC = example
APP = lmdbxx_sample
PROJ_TAG = demo

CPPFLAGS = -I$(includedir)/util/lmdbxx $(LMDB_INCLUDE) $(ORIG_CPPFLAGS)

LIB  = $(LMDB_LIB)
LIBS = $(LMDB_LIBS) $(ORIG_LIBS)

WATCHERS = ivanov
