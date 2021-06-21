# $Id: Makefile.remote_cgi.app 545644 2017-09-08 14:48:58Z sadyrovr $
# Author:  Maxim Didneko
#################################

PROJ_TAG = grid

APP = remote_cgi
SRC = remote_cgi_wn exec_helpers

REQUIRES = MT

LIB = xconnserv xthrserv xconnect xcgi xutil xncbi 
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = sadyrovr
