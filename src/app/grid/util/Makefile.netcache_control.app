#################################
# $Id: Makefile.netcache_control.app 466155 2015-04-29 14:36:10Z sadyrovr $
#################################

APP = netcache_control
SRC = netcache_control

LIB = ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = sadyrovr