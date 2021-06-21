# $Id: Makefile.remote_app_client_sample.app 466155 2015-04-29 14:36:10Z sadyrovr $

APP = remote_app_client_sample
SRC = remote_app_client_sample

### BEGIN COPIED SETTINGS

LIB = xconnserv xthrserv xconnect xutil xncbi 

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

### END COPIED SETTINGS

WATCHERS = sadyrovr
