# $Id: Makefile.test_grid_worker.app 466155 2015-04-29 14:36:10Z sadyrovr $

APP = test_grid_worker
SRC = test_grid_worker
LIB = xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

CHECK_COPY = test_grid_worker.ini

WATCHERS = sadyrovr
