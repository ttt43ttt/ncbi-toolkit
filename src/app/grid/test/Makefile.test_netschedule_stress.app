# $Id: Makefile.test_netschedule_stress.app 507679 2016-07-20 20:08:08Z satskyse $

APP = test_netschedule_stress
SRC = test_netschedule_stress
LIB = xconnserv xconnect xutil xncbi

LIBS = $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


WATCHERS = satskyse
