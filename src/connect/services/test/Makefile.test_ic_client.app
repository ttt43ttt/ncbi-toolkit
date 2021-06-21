# $Id: Makefile.test_ic_client.app 496104 2016-03-23 19:04:45Z sadyrovr $

CPPFLAGS = $(BOOST_INCLUDE) $(ORIG_CPPFLAGS)

APP = test_ic_client
SRC = test_ic_client
LIB = ncbi_xcache_netcache xconnserv xthrserv xconnect xutil test_boost xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included

CHECK_REQUIRES = in-house-resources
CHECK_CMD = test_ic_client
CHECK_TIMEOUT = 400

WATCHERS = sadyrovr gouriano fukanchi
