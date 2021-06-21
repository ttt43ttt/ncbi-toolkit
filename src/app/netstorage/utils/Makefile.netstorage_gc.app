#################################
# $Id: Makefile.netstorage_gc.app 536927 2017-05-24 16:00:07Z ucko $
#################################

WATCHERS = satskyse

APP = netstorage_gc
SRC = netstorage_gc netstorage_gc_database netstorage_gc_exception

REQUIRES = MT Linux


LIB =  netstorage ncbi_xcache_netcache xconnserv xthrserv \
       $(SDBAPI_LIB) xconnect xutil xncbi
LIBS = $(SDBAPI_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS)
