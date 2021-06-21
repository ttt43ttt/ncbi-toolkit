# $Id: Makefile.grid_worker_sample.app 545644 2017-09-08 14:48:58Z sadyrovr $

PROJ_TAG = grid

APP = grid_worker_sample
SRC = grid_worker_sample

### BEGIN COPIED SETTINGS

LIB = xconnserv xthrserv xconnect xutil xncbi 

REQUIRES = MT

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

### END COPIED SETTINGS

WATCHERS = sadyrovr
