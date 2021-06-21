# $Id: Makefile.logs_replay.app 503218 2016-06-01 22:19:14Z vakatov $

APP = logs_replay
SRC = logs_replay
LIB = ncbi_xcache_netcache xconnserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

#REQUIRES = MT
REQUIRES = MT Linux

WATCHERS = gouriano
