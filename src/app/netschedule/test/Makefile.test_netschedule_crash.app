# $Id: Makefile.test_netschedule_crash.app 507674 2016-07-20 19:49:11Z satskyse $

APP = test_netschedule_crash
SRC = test_netschedule_crash
LIB = xconnserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)
REQUIRES = Linux

WATCHERS = satskyse
