# $Id: Makefile.test_gridclient_stress.app 466155 2015-04-29 14:36:10Z sadyrovr $

APP = test_gridclient_stress
SRC = test_gridclient_stress
LIB = xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_COPY = test_gridclient_stress.ini

WATCHERS = sadyrovr
