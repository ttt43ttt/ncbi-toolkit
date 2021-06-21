# $Id: Makefile.remote_app.app 545644 2017-09-08 14:48:58Z sadyrovr $
# Author:  Maxim Didneko
#################################

PROJ_TAG = grid

APP = remote_app
SRC = remote_app_wn exec_helpers

REQUIRES = MT

LIB = xconnserv xthrserv xconnect xutil xncbi 
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = sadyrovr
