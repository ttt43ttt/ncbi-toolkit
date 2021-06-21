# $Id: Makefile.check_nc_mirror.app 423581 2014-01-06 22:18:19Z rafanovi $

APP = check_nc_mirror
SRC = check_nc_mirror
LIB = xconnserv xthrserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)

WATCHERS = rafanovi
