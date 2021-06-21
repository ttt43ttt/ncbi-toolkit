# $Id: Makefile.grid_client_sample.app 545644 2017-09-08 14:48:58Z sadyrovr $

PROJ_TAG = grid

APP = grid_client_sample
SRC = grid_client_sample

### BEGIN COPIED SETTINGS

LIB = xconnserv xthrserv xconnect xutil xncbi 

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

### END COPIED SETTINGS

WATCHERS = sadyrovr
