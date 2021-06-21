# $Id: Makefile.test_netschedule_node.app 507678 2016-07-20 20:06:10Z satskyse $

APP = test_netschedule_node
SRC = test_netschedule_node
LIB = xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = MT

WATCHERS = satskyse
