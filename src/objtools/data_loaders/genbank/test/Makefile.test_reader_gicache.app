# $Id: Makefile.test_reader_gicache.app 539764 2017-06-27 15:27:31Z ucko $

REQUIRES = unix LMDB

APP = test_reader_gicache
SRC = test_reader_gicache
LIB = ncbi_xreader_gicache $(LMDB_LIB) $(OBJMGR_LIBS)

LIBS = $(LMDB_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

//CHECK_CMD = test_reader_gicache

WATCHERS = vasilche
