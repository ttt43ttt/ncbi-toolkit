# $Id: Makefile.test_netstorage.app 530906 2017-03-19 01:27:34Z lavr $

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

APP = test_netstorage
SRC = test_netstorage
LIB = netstorage ncbi_xcache_netcache xconnserv \
        xthrserv xconnect xutil test_boost xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_netstorage
CHECK_COPY = test_netstorage.ini
CHECK_TIMEOUT = 3000

WATCHERS = sadyrovr fukanchi
