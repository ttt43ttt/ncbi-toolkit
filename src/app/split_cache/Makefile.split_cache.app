#################################
# $Id: Makefile.split_cache.app 500573 2016-05-05 16:42:41Z gouriano $
# Author:  Eugene Vasilchenko
#################################

# Build application for splitting blobs withing ID1 cache
#################################

REQUIRES = bdb BerkeleyDB objects -Cygwin

APP = split_cache
SRC = split_cache
LIB = id2_split $(BDB_CACHE_LIB) $(BDB_LIB) $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(BERKELEYDB_LIBS) $(ORIG_LIBS)

#CHECK_CMD = test_split_cache.sh
CHECK_COPY = test_split_cache.sh
CHECK_TIMEOUT = 1000

WATCHERS = vasilche