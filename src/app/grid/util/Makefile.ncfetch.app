#################################
# $Id: Makefile.ncfetch.app 545644 2017-09-08 14:48:58Z sadyrovr $
#################################

PROJ_TAG = grid

APP = ncfetch.cgi
SRC = ncfetch

LIB = xcgi xconnserv xthrserv xconnect xutil xncbi
LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = sadyrovr
