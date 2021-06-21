# $Id: Makefile.ns_loader.app 507673 2016-07-20 19:47:23Z satskyse $

APP = ns_loader
SRC = ns_loader
LIB = xconnserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
REQUIRES = Linux

WATCHERS = satskyse
