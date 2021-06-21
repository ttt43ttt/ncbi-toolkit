# $Id: Makefile.bdb_env_keeper.app 507684 2016-07-20 20:21:30Z satskyse $

WATCHERS = vakatov

APP = bdb_env_keeper
SRC = bdb_env_keeper
LIB = xconnect xutil xncbi


LIB =  $(BDB_LIB) $(COMPRESS_LIBS) xconnserv xthrserv xconnect xutil xncbi
LIBS = $(BERKELEYDB_LIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


REQUIRES = MT bdb

